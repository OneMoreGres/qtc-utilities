#include "ClassDiagramGenerator.h"

#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>

#include <cpptools/cppmodelmanager.h>
#include <cpptools/typehierarchybuilder.h>
#include <cpptools/symbolfinder.h>

#include <utils/qtcassert.h>

using namespace CppTools;
using namespace CPlusPlus;

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

enum Relation {
  Extension, Dependency, Association
};

class Generator
{
  public:
    explicit Generator (ClassFlags f);
    QString operator () (Symbol *symbol);

  private:
    void processHierarchy (const TypeHierarchy &hierarchy);
    void processClass (const Class *c, const QList<TypeHierarchy> &hierarchy);
    void processEnum (const Enum *e);

    QString classView (const Class *c);
    bool isInterface (const Class *c) const;
    QString relation (const Symbol *l, Relation relation, const Symbol *r,
                      const QString &comment = {}) const;
    QString namespacedName (const Symbol *s) const;

    QString accessType (const Symbol *s) const;
    QString abstractStaticType (const Symbol *s) const;
    QString member (Symbol *s, bool showDetails);
    void processDependency (const QString &dependency, Class *dependant, Symbol *scope);

    Symbol * find (const QString &name, Symbol *baseScope) const;
    QString symbolFile (const Symbol *s) const;
    bool fromSameProject (const Symbol *s) const;
    void addToSelectedHierarchy (const TypeHierarchy &hierarchy);

    ClassFlags flags_;
    Snapshot snapshot_;
    QMap<Relation, QString> relationNames_;
    Overview o_;
    QStringList used_;
    QStringList classes_;
    QStringList relations_;
    QStringList projectFiles_;

    Symbol *selected_;
    QSet<const Symbol *> selectedHierarchy_;
    SymbolFinder *finder_;
};

Generator::Generator (ClassFlags flags) :
  flags_ (flags), snapshot_ (CppModelManager::instance ()->snapshot ()),
  relationNames_ {{Extension, QStringLiteral ("<|--")},
                  {Dependency, QStringLiteral ("<...")},
                  {Association, QStringLiteral ("--")}},
  selected_ (nullptr), finder_ (CppModelManager::instance ()->symbolFinder ())
{
}

QString Generator::operator () (Symbol *symbol)
{
  for (auto part: CppModelManager::instance ()->projectPart (symbolFile (symbol))) {
    for (auto file: part->files) {
      projectFiles_ << file.path;
    }
  }

  selected_ = symbol;
  selectedHierarchy_ << symbol;

  auto builder = TypeHierarchyBuilder (symbol, snapshot_);
  auto hierarchy = builder.buildDerivedTypeHierarchy ();
  addToSelectedHierarchy (hierarchy);
  processHierarchy (hierarchy);

  QStringList parts;
  parts << QStringLiteral ("@startuml");
  QMap<QString, QString> skins = {
    {QStringLiteral ("selected"), QStringLiteral ("PaleGreen")},
    {QStringLiteral ("hierarchy"), QStringLiteral ("PaleTurquoise")}
  };
  for (const auto &type: skins.keys ()) {
    parts << QString (QStringLiteral ("skinparam class {\nBackgroundColor<<%1>> %2\n}"))
      .arg (type, skins[type]);
    parts << QString (QStringLiteral ("skinparam interface {\nBackgroundColor<<%1>> %2\n}"))
      .arg (type, skins[type]);
  }
  parts << QStringLiteral ("set namespaceSeparator ::");
  parts += classes_;
  parts += relations_;
  parts << QStringLiteral ("@enduml");
  parts.removeDuplicates ();
  return parts.join (QStringLiteral ("\n"));
}


FullySpecifiedType finalType (FullySpecifiedType t)
{
  auto type = t;
  while (auto *p = type->asPointerType ()) {
    type = p->elementType ();
  }
  return type;
}

bool isClassLike (const Symbol *s)
{
  auto type = finalType (s->type ());
  return (type->isClassType () || type->isEnumType ());
}

Symbol * Generator::find (const QString &name, Symbol *baseScope) const
{
  if (isClassLike (baseScope)) {
    return baseScope;
  }
  auto document = snapshot_.document (QString::fromUtf8 (baseScope->fileName ()));
  auto *scope = document->scopeAt (baseScope->line ());
  TypeOfExpression toe_;
  toe_.init (document, snapshot_);
  auto items = toe_ (name.toUtf8 (), scope);
  Symbol *forward = nullptr;
  for (auto item: items) {
    if (auto *d = item.declaration ()) {
      if (auto *t = d->asTemplate ()) {
        d = t->declaration ();
      }
      if (isClassLike (d)) {
        return d;
      }
      if (!forward && o_ (d->name ()) == name) {
        forward = d->asForwardClassDeclaration ();
      }
    }
  }
  if (forward) {
    auto *d = finder_->findMatchingClassDeclaration (forward, snapshot_);
    if (d && isClassLike (d)) {
      return d;
    }
  }
  return nullptr;
}

QString Generator::symbolFile (const Symbol *s) const
{
  return QString::fromUtf8 (s->fileName (), int (s->fileNameLength ()));
}

bool Generator::fromSameProject (const Symbol *s) const
{
  return projectFiles_.contains (symbolFile (s));
}

void Generator::addToSelectedHierarchy (const TypeHierarchy &hierarchy)
{
  auto *symbol = hierarchy.symbol ();
  selectedHierarchy_ << symbol;

  if (flags_ & ShowBase) {
    if (auto *c = symbol->asClass ()) {
      for (uint i = 0, end = c->baseClassCount (); i < end; ++i) {
        auto *base = c->baseClassAt (i);
        if (auto *s = find (o_ (base->name ()), base)) {
          addToSelectedHierarchy (s);
        }
      }
    }
  }

  if (flags_ & ShowDerived) {
    for (const auto &subHierarchy: hierarchy.hierarchy ()) {
      addToSelectedHierarchy (subHierarchy);
    }
  }
}

void Generator::processHierarchy (const TypeHierarchy &hierarchy)
{
  auto *symbol = hierarchy.symbol ();
  auto name = o_ (symbol->name ());
  if (used_.contains (name)) {
    return;
  }
  used_ << name;

  auto type = finalType (symbol->type ());
  if (auto *c = type->asClassType ()) {
    processClass (c, hierarchy.hierarchy ());
  }
  else if (auto *e = type->asEnumType ()) {
    processEnum (e);
  }
}

void Generator::processClass (const Class *c, const QList<TypeHierarchy> &hierarchy)
{
  classes_ << classView (c);

  if (flags_ & ShowBase) {
    for (uint i = 0, end = c->baseClassCount (); i < end; ++i) {
      auto *base = c->baseClassAt (i);
      if (auto *processible = find (o_ (base->name ()), base)) {
        processHierarchy (processible);
        relations_ << relation (processible, Extension, c);
      }
    }
  }

  if (flags_ & ShowDerived) {
    for (const auto &subHierarchy: hierarchy) {
      processHierarchy (subHierarchy);
      auto *s = subHierarchy.symbol ();
      if (isClassLike (s)) {
        relations_ << relation (c, Extension, s);
      }
    }
  }
}

QString Generator::classView (const Class *c)
{
  QStringList lines;
  auto type = isInterface (c) ? QStringLiteral ("interface") : QStringLiteral ("class");

  QString stereotype;
  if (selected_ == c) {
    stereotype = QStringLiteral ("<<selected>>");
  }
  else if (selectedHierarchy_.contains (c)) {
    stereotype = QStringLiteral ("<<hierarchy>>");
  }

  lines << QString (QStringLiteral ("%1 %2%3 {")).arg (type, namespacedName (c),
                                                       stereotype);
  auto inHierarchy = selectedHierarchy_.contains (c);
  auto showMemberDetails =
    ((flags_ & ShowHierarchyDetails) && inHierarchy)
    || c == selected_
    || ((flags_ & ShowDependsDetails) && !inHierarchy)
    || ((flags_ & ShowDetailsFromSameProject) && fromSameProject (c));
  for (uint i = 0, end = c->memberCount (); i < end; ++i) {
    lines << member (c->memberAt (i), showMemberDetails);
  }
  lines << QStringLiteral ("}");
  return lines.join (QStringLiteral ("\n"));
}

bool Generator::isInterface (const Class *c) const
{
  for (uint i = 0, end = c->memberCount (); i < end; ++i) {
    auto *symbol = c->memberAt (i);
    if (auto *f = symbol->type ()->asFunctionType ()) {
      if (f->isPureVirtual ()) {
        return true;
      }
    }
  }
  return false;
}

QString Generator::relation (const Symbol *l, Relation relation, const Symbol *r,
                             const QString &comment) const
{
  return namespacedName (l)
         + relationNames_.value (relation, QStringLiteral ("--"))
         + namespacedName (r)
         + (comment.isEmpty () ? comment : QStringLiteral (": ") + comment);
}

QString Generator::namespacedName (const Symbol *s) const
{
  QStringList classes;
  auto name = o_ (s->name ());
  if (name.contains (QStringLiteral ("<"))) {
    name.replace (QStringLiteral ("<"), QStringLiteral ("_"));
    name.replace (QStringLiteral (">"), QStringLiteral ("_"));
  }
  if (name.contains (QStringLiteral (":"))) {
    name.replace (QStringLiteral (":"), QStringLiteral ("_"));
  }
  classes << name;

  for (auto *i = s->enclosingClass (); i; i = i->enclosingClass ()) {
    classes.prepend (o_ (i->name ()));
  }
  classes.removeAll ({});

  QStringList namespaces;
  namespaces << classes.join (QStringLiteral ("."));
  for (auto *i = s->enclosingNamespace (); i; i = i->enclosingNamespace ()) {
    namespaces.prepend (o_ (i->name ()));
  }
  namespaces.removeAll ({});

  return namespaces.join (QStringLiteral ("::"));
}

QString Generator::accessType (const Symbol *s) const
{
  if (s->isPublic () && (flags_ & ShowPublic)) {
    return QStringLiteral ("+");
  }
  else if (s->isProtected () && (flags_ & ShowProtected)) {
    return QStringLiteral ("#");
  }
  else if (s->isPrivate () && (flags_ & ShowPrivate)) {
    return QStringLiteral ("-");
  }
  else {
    return {};
  }
}

QString Generator::abstractStaticType (const Symbol *s) const
{
  auto type = s->type ();
  if (type.isStatic ()) {
    return QStringLiteral ("{static}");
  }
  else if (type.isVirtual ()) {
    return QStringLiteral ("{abstract}");
  }
  return {};
}

void Generator::processEnum (const Enum *e)
{
  QStringList lines;
  lines << QString (QStringLiteral ("enum %1 {")).arg (namespacedName (e));
  if (flags_ & ShowDependsDetails) {
    for (uint i = 0, end = e->memberCount (); i < end; ++i) {
      lines << o_ (e->memberAt (i)->name ());
    }
  }
  lines << QStringLiteral ("}");
  classes_ << lines.join (QStringLiteral ("\n"));
}

QString Generator::member (Symbol *s, bool showDetails)
{
  auto access = accessType (s);
  auto notFilteredByAccess = !access.isEmpty ();
  auto lead = QString (access + abstractStaticType (s));
  auto type = finalType (s->type ());
  auto name = o_ (s->name ());

  if (auto *f = type->asFunctionType ()) {
    auto returnType = finalType (f->returnType ());
    if ((flags_ & ShowDependencies) && returnType->isNamedType ()) {
      processDependency (o_ (returnType), s->enclosingClass (), s);
    }
    if ((flags_ & ShowMethods) && notFilteredByAccess && showDetails && !f->isFriend ()) {
      return QString (QStringLiteral ("%1 %2 %3: %4"))
             .arg (lead, name, o_ (s->type ()), o_ (f->returnType ()));
    }
    return {};
  }

  if (s->isDeclaration ()) {
    if ((flags_ & ShowDependencies) && type->isNamedType ()) {
      processDependency (o_ (type), s->enclosingClass (), s);
    }
    if ((flags_ & ShowMembers) && notFilteredByAccess && showDetails) {
      return QString (QStringLiteral ("%1 %2: %3")).arg (lead, name, o_ (s->type ()));
    }
    return {};
  }

  if ((flags_ & ShowNested) && isClassLike (s)) {
    processHierarchy (s);
    relations_ << relation (s, Association, s->enclosingClass (),
                            QStringLiteral ("nested >"));
    return {};
  }
  return {};
}

void Generator::processDependency (const QString &dependency, Class *dependant,
                                   Symbol *scope)
{
  if ((flags_ & ShowOnlyHierarchyDependencies) && !selectedHierarchy_.contains (dependant)) {
    return;
  }
  if (auto *d = find (dependency, scope)) {
    processHierarchy (d);
    relations_ << relation (d, Dependency, dependant);
  }

  if (dependency.contains (QStringLiteral ("<"))) {
    auto begin = dependency.indexOf (QStringLiteral ("<"));
    auto end = dependency.indexOf (QStringLiteral (">"));
    if (begin != -1 && end != -1) {
      auto arguments = dependency.mid (begin + 1, end - begin - 1);
      for (const auto &i: arguments.split (QStringLiteral (","))) {
        if (auto *processable = find (i, scope)) {
          processHierarchy (processable);
          relations_ << relation (processable, Dependency, dependant);
        }
      }
    }
  }

}




ClassDiagramGenerator::ClassDiagramGenerator (QObject *parent) : QObject (parent)
{
}

QString ClassDiagramGenerator::generate (Symbol *symbol, ClassFlags flags) const
{
  QTC_ASSERT (symbol, {});
  QString source;
  if (isClassLike (symbol)) {
    Generator generator (flags);
    source = generator.operator () (symbol);
  }
  return source;
}

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

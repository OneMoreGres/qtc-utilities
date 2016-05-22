#include "IncludesExtractor.h"
#include "Document.h"

#include <extensionsystem/pluginmanager.h>

using namespace Core;
using namespace CppTools;
using namespace CPlusPlus;

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

IncludesExtractor::IncludesExtractor (const Document &document, bool useLocator)
  : ASTVisitor (document.translationUnit ()),
  document_ (document), locatorFilter_ (useLocator ? getLocatorFilter () : nullptr)
{
  expressionType_.init (document.cppDocument (), document.snapshot ());
}


ILocatorFilter * IncludesExtractor::getLocatorFilter () const
{
  return ExtensionSystem::PluginManager::getObject<Core::ILocatorFilter> (
    [](Core::ILocatorFilter *i) {
          return (i->id () == Core::Id ("Classes and Methods"));
        });
}


Includes IncludesExtractor::operator () ()
{
  usages_.clear ();
  if (!translationUnit () || !translationUnit ()->ast ()) {
    return usages_;
  }
  accept (translationUnit ()->ast ());
  usages_.removeAll (document_.file ());
  return usages_;
}


void IncludesExtractor::addUsage (const QString &file)
{
  Include include (file);
  if (!usages_.contains (include)) {
    usages_ << include;
  }
}


QString IncludesExtractor::fileNameViaLocator (const QString &name, int types)
{
  if (locatorFilter_) {
    QFutureInterface<LocatorFilterEntry> dummy;
    auto matches = locatorFilter_->matchesFor (dummy, name);

    QString overscopedName = QStringLiteral ("::") + name;
    auto match = std::find_if (matches.cbegin (), matches.cend (),
                               [&name, types, overscopedName](const LocatorFilterEntry &i) {
            auto item = i.internalData.value<IndexItem::Ptr>();
            if (!(item->type () & types)) {
              return false;
            }
            auto fullName = item->scopedSymbolName ();
            return fullName == name || fullName.endsWith (overscopedName);
          });

    if (match != matches.end ()) {
      auto item = (*match).internalData.value<IndexItem::Ptr>();
      qDebug () << "found via locator" << item->scopedSymbolName () << item->fileName ();
      return item->fileName ();
    }
  }
  return {};
}


bool IncludesExtractor::addType (const QString &typeName, Scope *scope)
{
  auto added = false;
  auto matches = expressionType_ (typeName.toUtf8 (), scope);
  for (const auto &i: matches) {
    if (const auto declaration = i.declaration ()) {
      if (declaration->isForwardClassDeclaration ()) {
        continue;
      }
      auto fileName = QString::fromUtf8 (declaration->fileName ());
      addUsage (fileName);
      added = true;
      qDebug () <<  " found type" << typeName << "at" << fileName;
    }
  }
  return added;
}


void IncludesExtractor::addViaLocator (const QString &name, int types)
{
  auto fileName = fileNameViaLocator (name, types);
  if (!fileName.isEmpty ()) {
    addUsage (fileName);
  }
}

bool IncludesExtractor::addTypedItems (const QList<LookupItem> &items,
                                       const QString &name, Scope *scope)
{
  auto added = false;
  for (const auto &i: items) {
    if (const auto declaration = i.declaration ()) {
      if (declaration->isForwardClassDeclaration ()) {
        continue;
      }
      auto fileName = QString::fromUtf8 (declaration->fileName ());
      addUsage (fileName);
      qDebug () <<  " found" << name << "at" << fileName;
      auto typeName = overview_ (i.type ().type ());
      if (!typeName.isEmpty ()) {
        addType (typeName, scope);
      }
      added = true;
    }
  }
  return added;
}


bool IncludesExtractor::visit (NamedTypeSpecifierAST *ast)
{
  auto scope = document_.scopeAtToken (ast->firstToken ());
  auto typeName = overview_ (ast->name->name);
  if (typeName.isEmpty () || !addType (typeName, scope)) {
    addViaLocator (typeName, IndexItem::Enum | IndexItem::Class);
  }
  return true;
}


bool IncludesExtractor::visit (DeclaratorIdAST *ast)
{
  auto scope = document_.scopeAtToken (ast->firstToken ());
  auto typeName = overview_ (ast->name->name);
  auto matches = expressionType_ (typeName.toUtf8 (), scope);
  addTypedItems (matches, typeName, scope);
  return true;
}


bool IncludesExtractor::visit (CallAST *ast)
{
  auto scope = document_.scopeAtToken (ast->firstToken ());
  QString callName;
  if (auto e = ast->base_expression->asIdExpression ()) {
    callName = overview_ (e->name->name);
  }
  else {
    if (auto e = ast->base_expression->asMemberAccess ()) {
      callName = overview_ (e->member_name->name);
    }
  }
  auto matches = expressionType_ (ast->base_expression, document_.cppDocument (), scope);
  if (!addTypedItems (matches, callName, scope)) {
    addViaLocator (callName, IndexItem::All);
  }
  return true;
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

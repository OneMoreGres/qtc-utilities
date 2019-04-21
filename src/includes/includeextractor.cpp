#include "includeextractor.h"

#include <extensionsystem/pluginmanager.h>

#include <cplusplus/LookupContext.h>

#include <utils/qtcassert.h>

using namespace Core;
using namespace CppTools;
using namespace CPlusPlus;

IncludeExtractor::IncludeExtractor (Document::Ptr document,
                                    const Snapshot &snapshot, bool useLocator) :
  ASTVisitor (document ? document->translationUnit () : nullptr),
  document_ (document),
  locatorFilter_ (nullptr) {

  if (!translationUnit () || !translationUnit ()->ast ()) {
    return;
  }

  if (useLocator) {
    initLocatorFilter ();
  }

  expressionType_.init (document, snapshot);

  accept (translationUnit ()->ast ());

  //  Scope *previousScope = switchScope(_doc->globalNamespace());
  //  for (DeclarationListAST *it = ast->declaration_list; it; it = it->next) {
  //      this->declaration(it->value);
  //  }
}

void IncludeExtractor::initLocatorFilter () {
  //  locatorFilter_ = CppModelManager::instance ()->classesFilter ();
}


bool IncludeExtractor::visit (NamedTypeSpecifierAST *ast) {
  const auto typeName = overview_ (ast->name->name);
  qDebug () << "NamedTypeSpecifierAST" << typeName;
  if (typeName.isEmpty ()) {
    return true;
  }

  const auto scope = scopeAtToken (ast->firstToken ());
  const auto matches = expressionType_ (typeName.toUtf8 (), scope);

  for (const auto &match: matches) {
    add (match);
  }

  return true;
}


bool IncludeExtractor::visit (DeclaratorIdAST *ast) {
  const auto typeName = overview_ (ast->name->name);
  qDebug () << "DeclaratorIdAST" << typeName;
  if (typeName.isEmpty ()) {
    return true;
  }

  const auto scope = scopeAtToken (ast->firstToken ());
  const auto matches = expressionType_ (typeName.toUtf8 (), scope);

  for (const auto &match: matches) {
    qDebug () << overview_ (match.type ()) << match.declaration ();
    add (match);

    expressionType_ (overview_ (match.type ()).toUtf8 (), match.scope ());
    accept (expressionType_.ast ());
  }

  return true;
}

bool IncludeExtractor::visit (IdExpressionAST *ast) {
  static auto noRecursion = false;
  const auto typeName = overview_ (ast->name->name);
  qDebug () << "IdExpressionAST" << typeName;
  if (typeName.isEmpty ()) {
    return true;
  }

  const auto scope = scopeAtToken (ast->firstToken ());
  const auto matches = expressionType_ (typeName.toUtf8 (), scope);

  for (const auto &match: matches) {
    qDebug () << overview_ (match.type ()) << match.declaration ();
    add (match);

    if (!noRecursion) {
      noRecursion = true;
      expressionType_ (overview_ (match.type ()).toUtf8 (), match.scope ());
      accept (expressionType_.ast ());
      noRecursion = false;
    }
  }

  return true;
}

bool IncludeExtractor::visit (CallAST *ast) {
  qDebug () << "CallAST";
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (ast->base_expression, return true);
  addExpression (ast->base_expression, scope);

  auto expression = ast->expression_list;
  while (expression) {
    addExpression (expression->value, scope);
    expression = expression->next;
  }

  return true;
}

bool IncludeExtractor::visit (TemplateIdAST *ast) {
  const auto typeName = overview_ (ast->name);
  qDebug () << "TemplateIdAST" << typeName;
  if (typeName.isEmpty ()) {
    return true;
  }

  const auto scope = scopeAtToken (ast->firstToken ());
  const auto matches = expressionType_ (typeName.toUtf8 (), scope);

  for (const auto &match: matches) {
    add (match);
  }

  return true;
}

bool IncludeExtractor::visit (UsingDirectiveAST *ast) {
  const auto typeName = overview_ (ast->name->name);
  qDebug () << "UsingDirectiveAST" << typeName;
  if (typeName.isEmpty ()) {
    return true;
  }

  const auto scope = scopeAtToken (ast->firstToken ());
  const auto matches = expressionType_ (typeName.toUtf8 (), scope);

  for (const auto &match: matches) {
    add (match);
  }

  return true;
}

Scope *IncludeExtractor::scopeAtToken (unsigned token) const {
  QTC_ASSERT (translationUnit (), return nullptr);
  unsigned line = 0;
  unsigned column = 0;
  translationUnit ()->getTokenStartPosition (token, &line, &column);
  return document_->scopeAt (line);
}

QString IncludeExtractor::fileNameViaLocator (const QString &name, int types) {
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

bool IncludeExtractor::add (const LookupItem &lookup) {
  const auto declaration = lookup.declaration ();
  if (!declaration
      || document_->fileName () == QString::fromUtf8 (declaration->fileName ())) {
    return false;
  }

  QTC_ASSERT (declaration->fileName (), return false);
  const auto fileName = QString::fromUtf8 (declaration->fileName ());
  includes_.insert (fileName);
  symbols_.insert (declaration);

  qDebug () <<  " add"
            << overview_ (lookup.type ().type ())
            << "at" << fileName
            << declaration
            << typeid (*declaration).name ()
            << overview_ (declaration->type ().type ());

  return true;
}


//bool IncludeExtractor::addType (const QString &typeName, Scope *scope) {
//  auto added = false;

//  const auto matches = expressionType_ (typeName.toUtf8 (), scope);

//  for (const auto &match: matches) {
//    const auto declaration = match.declaration ();
//    if (!declaration
//        || declaration->isForwardClassDeclaration ()) {
//      continue;
//    }

//    QTC_ASSERT (declaration->fileName (), continue);
//    const auto fileName = QString::fromUtf8 (declaration->fileName ());
//    includes_.insert (fileName);
//    added = true;

//    qDebug () <<  " found type" << typeName << "at" << fileName
//              << overview_ (match.type ().type ())
//              << overview_ (declaration->type ().type ());
//    if (declaration->isTemplate ()) {
//      break;
//    }
//  }

//  return added;
//}

//bool IncludeExtractor::addTypedItems (const QList<LookupItem> &items,
//                                      const QString &name, Scope *scope) {
//  auto added = false;

//  for (const auto &match: items) {
//    const auto declaration = match.declaration ();
//    if (!declaration || declaration->isForwardClassDeclaration ()) {
//      continue;
//    }

//    QTC_ASSERT (declaration->fileName (), continue);
//    const auto fileName = QString::fromUtf8 (declaration->fileName ());
//    includes_.insert (fileName);
//    added = true;

//    qDebug () <<  " found" << name << "at" << fileName;
//    auto typeName = overview_ (match.type ().type ());
//    if (!typeName.isEmpty ()) {
//      addType (typeName, scope);
//    }
//  }

//  return added;
//}

const QSet<QString> &IncludeExtractor::includes () const {
  return includes_;
}

void IncludeExtractor::addExpression (ExpressionAST *ast, Scope *scope) {
  QString callName;
  if (auto e = ast->asIdExpression ()) {
    callName = overview_ (e->name->name);
  }
  else if (auto e = ast->asMemberAccess ()) {
    callName = overview_ (e->member_name->name);
  }
  qDebug () << "addExpression" << callName;

  const auto matches = expressionType_ (ast, document_, scope);
  for (const auto &match: matches) {
    add (match);
  }

  //  if (!addTypedItems (matches, callName, scope) && !callName.isEmpty ()
  //      && !ast->asMemberAccess ()) {
  //    addViaLocator (callName, IndexItem::All);
  //  }
}

void IncludeExtractor::addViaLocator (const QString &name, int types) {
  auto fileName = fileNameViaLocator (name, types);
  if (!fileName.isEmpty ()) {
    includes_.insert (fileName);
  }
}

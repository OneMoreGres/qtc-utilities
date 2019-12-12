#include "includeextractor.h"

#include <extensionsystem/pluginmanager.h>

#include <cplusplus/LookupContext.h>

#include <utils/qtcassert.h>

using namespace CPlusPlus;

static bool hasNonForwardDeclaration (const QList<LookupItem> &matches) {
  auto ok = false;
  for (const auto &i: matches) {
    ok |= (i.declaration () && !i.declaration ()->isForwardClassDeclaration ());
  }
  return ok;
}

IncludeExtractor::IncludeExtractor (Document::Ptr document,
                                    const Snapshot &snapshot) :
  ASTVisitor (document ? document->translationUnit () : nullptr),
  document_ (document),
  snapshot_ (snapshot),
  bindings_ (new CreateBindings (document, snapshot)) {
  QTC_ASSERT (document, return );

  //  bindings_->setExpandTemplates (true);

  if (!translationUnit () || !translationUnit ()->ast ()) {
    return;
  }

  accept (translationUnit ()->ast ());
}

bool IncludeExtractor::visit (NamedTypeSpecifierAST *ast) {
  QTC_ASSERT (ast, return false);
  if (!ast->name || !ast->name->name) {
    return true;
  }
  const auto name = Overview () (ast->name->name);
  qCritical () << "NamedTypeSpecifierAST" << name;
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  addExpression (name, scope);
  return true;
}

bool IncludeExtractor::visit (DeclaratorIdAST *ast) {
  QTC_ASSERT (ast, return false);
  if (!ast->name || !ast->name->name) {
    return true;
  }
  const auto name = Overview () (ast->name->name);
  qCritical () << "DeclaratorIdAST" << name;
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  addExpression (name, scope);
  return true;
}

bool IncludeExtractor::visit (IdExpressionAST *ast) {
  QTC_ASSERT (ast, return false);
  if (!ast->name || !ast->name->name) {
    return true;
  }
  const auto name = Overview () (ast->name->name);
  qCritical () << "IdExpressionAST" << name;
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  addExpression (name, scope);
  return true;
}

bool IncludeExtractor::visit (CallAST *ast) {
  QTC_ASSERT (ast, return false);
  qCritical () << "CallAST";
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  QTC_ASSERT (ast->base_expression, return true);
  addExpression (ast->base_expression, scope);

  auto expression = ast->expression_list;
  while (expression) {
    qCritical () << "CallAST expression_list";
    addExpression (expression->value, scope);
    expression = expression->next;
  }

  return true;
}

bool IncludeExtractor::visit (TemplateIdAST *ast) {
  QTC_ASSERT (ast, return false);
  if (!ast->name) {
    return true;
  }
  const auto name = Overview () (ast->name);
  qCritical () << "TemplateIdAST" << name;
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  addExpression (name, scope);
  return true;
}

bool IncludeExtractor::visit (UsingDirectiveAST *ast) {
  QTC_ASSERT (ast, return false);
  if (!ast->name || !ast->name->name) {
    return true;
  }
  const auto name = Overview () (ast->name->name);
  qCritical () << "UsingDirectiveAST" << name;
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  addExpression (name, scope);
  return true;
}

bool IncludeExtractor::visit (MemberAccessAST *ast) {
  QTC_ASSERT (ast, return false);
  if (!ast->member_name || !ast->member_name->name) {
    return true;
  }
  const auto name = Overview () (ast->member_name->name);
  qCritical () << "MemberAccessAST" << name;
  const auto scope = scopeAtToken (ast->firstToken ());
  QTC_ASSERT (scope, return true);
  addExpression (name, scope);
  return true;
}

Scope *IncludeExtractor::scopeAtToken (unsigned token) const {
  QTC_ASSERT (translationUnit (), return nullptr);
  int line = 0;
  int column = 0;
  translationUnit ()->getTokenStartPosition (token, &line, &column);
  return document_->scopeAt (line);
}

void IncludeExtractor::addExpression (const QString &name, CPlusPlus::Scope *scope) {
  if (name.isEmpty ()) {
    return;
  }
  QTC_ASSERT (scope, return );

  const auto pair = qMakePair (name, scope);
  if (checkedTypes_.contains (pair)) {
    return;
  }
  checkedTypes_.insert (pair);
  qCritical () << "add string expression" << name;

  TypeOfExpression toe;
  toe.init (document_, snapshot_, bindings_);
  const auto matches = toe (name.toUtf8 (), scope);

  addDeclarations (matches);

  if (toe.ast ()) {
    accept (toe.ast ());
  }
}

void IncludeExtractor::addExpression (ExpressionAST *ast, Scope *scope) {
  QTC_ASSERT (scope, return );
  QTC_ASSERT (ast, return );
  QString callName;
  if (auto e = ast->asIdExpression ()) {
    callName = Overview () (e->name->name);
  }
  else if (auto e = ast->asMemberAccess ()) {
    callName = Overview () (e->member_name->name);
  }
  qCritical () << "add ast expression" << callName;

  TypeOfExpression toe;
  toe.init (document_, snapshot_, bindings_);
  const auto matches = toe (ast, document_, scope);

  addDeclarations (matches);

  if (toe.ast ()) {
    accept (toe.ast ());
  }
}

void IncludeExtractor::addDeclarations (const QList<LookupItem> &declarations) {
  const auto hasNonForward = hasNonForwardDeclaration (declarations);

  for (const auto &match: declarations) {
    const auto matchType = Overview ()(match.type ());
    if (hasNonForward
        && match.declaration ()
        && match.declaration ()->isForwardClassDeclaration ()
        && !match.type ().isTypedef ()) {
      continue;
    }
    addDeclaration (match.declaration ());

    QTC_ASSERT (match.scope (), continue);
    addExpression (matchType, match.scope ());
  }
}

bool IncludeExtractor::addDeclaration (Symbol *declaration) {
  if (!declaration
      || document_->fileName () == QString::fromUtf8 (declaration->fileName ())) {
    return false;
  }

  QTC_ASSERT (declaration->fileName (), return false);
  const auto fileName = QString::fromUtf8 (declaration->fileName ());
  includes_.insert (fileName);
  symbols_.insert (declaration);

  qCritical () << ">>>addDeclaration"
               << "at" << fileName
               << declaration
               << typeid (*declaration).name ()
               << Overview () (declaration->type ().type ());

  return true;
}

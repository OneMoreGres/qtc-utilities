#pragma once

#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>

#include <cplusplus/CppDocument.h>

#include <QDebug>

class IncludeExtractor : public CPlusPlus::ASTVisitor {
  public:
    IncludeExtractor (CPlusPlus::Document::Ptr document,
                      const CPlusPlus::Snapshot &snapshot);

    bool visit (CPlusPlus::NamedTypeSpecifierAST *) override;
    bool visit (CPlusPlus::DeclaratorIdAST *) override;
    bool visit (CPlusPlus::IdExpressionAST *) override;
    bool visit (CPlusPlus::CallAST *) override;
    bool visit (CPlusPlus::TemplateIdAST *) override;
    bool visit (CPlusPlus::UsingDirectiveAST *) override;
    bool visit (CPlusPlus::MemberAccessAST *) override;

    const QSet<QString> &includes () const {
      return includes_;
    }
    const QSet<CPlusPlus::Symbol *> &symbols () const {
      return symbols_;
    }

  private:
    CPlusPlus::Scope *scopeAtToken (unsigned token) const;
    void addExpression (const QString &name, CPlusPlus::Scope *scope);
    void addExpression (CPlusPlus::ExpressionAST *ast, CPlusPlus::Scope *scope);
    void addDeclarations (const QList<CPlusPlus::LookupItem> &declarations);
    bool addDeclaration (CPlusPlus::Symbol *declaration);

  private:
    CPlusPlus::Document::Ptr document_;
    const CPlusPlus::Snapshot &snapshot_;
    QSharedPointer<CPlusPlus::CreateBindings> bindings_;
    QSet<QPair<QString, CPlusPlus::Scope *> > checkedTypes_;
    QSet<QString> includes_;
    QSet<CPlusPlus::Symbol *> symbols_;

#ifdef QT_DEBUG
    QString indent;

  public:
    bool preVisit (CPlusPlus::AST *a) override {
      qDebug () << qPrintable (indent) << "preVisit" << a << typeid(*a).name ();
      indent += QStringLiteral (" ");
      return true;
    }

    void postVisit (CPlusPlus::AST *) override {
      indent.chop (1);
    }
#endif
};


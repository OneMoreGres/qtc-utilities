#pragma once

#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>

#include <cpptools/cpplocatorfilter.h>

#include <cplusplus/CppDocument.h>

class IncludeExtractor : public CPlusPlus::ASTVisitor {
  public:
    IncludeExtractor (CPlusPlus::Document::Ptr document,
                      const CPlusPlus::Snapshot &snapshot,
                      bool useLocator);

    bool visit (CPlusPlus::NamedTypeSpecifierAST *) override;
    bool visit (CPlusPlus::DeclaratorIdAST *) override;
    bool visit (CPlusPlus::IdExpressionAST *) override;
    bool visit (CPlusPlus::CallAST *) override;
    bool visit (CPlusPlus::TemplateIdAST *) override;
    bool visit (CPlusPlus::UsingDirectiveAST *) override;

    const QSet<QString> &includes () const;
    const QSet<CPlusPlus::Symbol *> &symbols () const {
      return symbols_;
    }

  private:
    void initLocatorFilter ();
    CPlusPlus::Scope *scopeAtToken (unsigned token) const;
    void addViaLocator (const QString &name, int types);
    QString fileNameViaLocator (const QString &name, int types);
    bool add (const CPlusPlus::LookupItem &lookup);
    void addExpression (CPlusPlus::ExpressionAST *ast, CPlusPlus::Scope *scope);
    //    bool addType (const QString &typeName, CPlusPlus::Scope *scope);
    //    bool addTypedItems (const QList<CPlusPlus::LookupItem> &items,
    //                        const QString &name, CPlusPlus::Scope *scope);
    bool hasNonForwardDeclaration (const QList<CPlusPlus::LookupItem> &matches) const;

  private:
    CPlusPlus::Document::Ptr document_;
    Core::ILocatorFilter *locatorFilter_;
    CPlusPlus::TypeOfExpression expressionType_;
    CPlusPlus::Overview overview_;
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


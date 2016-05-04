#pragma once

#include "Include.h"

#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>

#include <cpptools/cpplocatorfilter.h>

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

//! Extracts usages of symbols/function calls with its declaration files.
class IncludesExtractor : public CPlusPlus::ASTVisitor
{
  public:
    IncludesExtractor (CPlusPlus::Document::Ptr document, const CPlusPlus::Snapshot &snapshot);

    Includes operator () ();

    bool visit (CPlusPlus::NamedTypeSpecifierAST *) override;
    bool visit (CPlusPlus::IdExpressionAST *) override;

  private:
    void addUsage (const QString &file);
    CPlusPlus::Scope * scopeAtToken (unsigned token);
    QString fileNameViaLocator (const QString &name, int types);
    Core::ILocatorFilter * getLocatorFilter () const;

  private:
    CPlusPlus::TypeOfExpression expressionType_;
    CPlusPlus::Overview overview_;
    Includes usages_;

    CPlusPlus::Document::Ptr document_;
    Core::ILocatorFilter *locatorFilter_;


#ifdef QT_DEBUG

  public:
    bool preVisit (CPlusPlus::AST *a) override
    {
      qDebug () << qPrintable (indent) << "preVisit" << a << typeid(*a).name ();
      indent += QStringLiteral (" ");
      return true;
    }

    void postVisit (CPlusPlus::AST *) override
    {
      indent = indent.mid (1);
    }

    QString indent;
#endif
};


} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

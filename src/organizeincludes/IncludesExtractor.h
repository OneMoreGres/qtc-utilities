#pragma once

#include "Include.h"

#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>

#include <cpptools/cpplocatorfilter.h>

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

class Document;

//! Extracts usages of symbols/function calls with its declaration files.
class IncludesExtractor : public CPlusPlus::ASTVisitor
{
  public:
    explicit IncludesExtractor (const Document &document);

    Includes operator () ();

    bool visit (CPlusPlus::NamedTypeSpecifierAST *) override;
    bool visit (CPlusPlus::DeclaratorIdAST *) override;
    bool visit (CPlusPlus::CallAST *) override;

  private:
    void addUsage (const QString &file);
    QString fileNameViaLocator (const QString &name, int types);
    Core::ILocatorFilter * getLocatorFilter () const;
    bool addType (const QString &typeName, CPlusPlus::Scope *scope);
    void addViaLocator (const QString &name, int types);
    bool addTypedItems (const QList<CPlusPlus::LookupItem> &items,
                        const QString &name, CPlusPlus::Scope *scope);

  private:
    CPlusPlus::TypeOfExpression expressionType_;
    CPlusPlus::Overview overview_;
    Includes usages_;

    const Document &document_;
    Core::ILocatorFilter *locatorFilter_;


#ifdef QT_DEBUG
    QString indent;

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
#endif
};


} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

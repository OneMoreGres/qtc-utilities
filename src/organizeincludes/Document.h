#pragma once

#include "Include.h"

#include <cplusplus/CppDocument.h>

#include <coreplugin/idocument.h>

#include <QTextDocument>

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

class Document
{
  public:
    explicit Document (Core::IDocument *idocument);

    bool isValid () const;
    Includes includes () const;
    void addInclude (const Include &include);
    QList<int> includeLines () const;
    QList<QString> includePaths () const;
    QString projectPath () const;
    QTextDocument * textDocument () const;

    CPlusPlus::Scope * scopeAtToken (unsigned token) const;
    int lineAfterFirstComment () const;

    QString file () const;

    const CPlusPlus::Snapshot &snapshot () const;
    CPlusPlus::Document::Ptr cppDocument () const;
    CPlusPlus::TranslationUnit * translationUnit () const;

  private:
    Core::IDocument *idocument_;
    CPlusPlus::Document::Ptr cppDocument_;
    CPlusPlus::Snapshot snapshot_;
};

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

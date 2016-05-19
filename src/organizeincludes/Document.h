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
    QStringList includePaths () const;
    QString projectPath () const;

    int lineForInclude (const Include &include) const;

    void replaceInclude (const Include &include);
    void removeInclude (const Include &include);
    void addInclude (const Include &include);
    void reorderIncludes (const Includes &includes);

    CPlusPlus::Scope * scopeAtToken (unsigned token) const;
    QString file () const;

    const CPlusPlus::Snapshot &snapshot () const;
    CPlusPlus::Document::Ptr cppDocument () const;
    CPlusPlus::TranslationUnit * translationUnit () const;

  private:
    void removeNewLinesBefore (int line, bool isNew);
    int realLine (int line, bool isNew) const;
    void addToCppDocument (const Include &include);
    QTextCursor cursor (int line, bool isNew = false);
    int lineAfterFirstComment () const;
    void unfoldDocument ();

    Core::IDocument *idocument_;
    CPlusPlus::Document::Ptr cppDocument_;
    CPlusPlus::Snapshot snapshot_;
    QTextDocument *textDocument_;
    QStringList includePaths_;
    QString projectPath_;
    QMap<int, int> lineChanges_;
};

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

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
    QList<int> includeLines () const;
    QList<QString> includePaths () const;
    QString projectPath () const;
    QTextDocument * textDocument () const;

    void replaceInclude (const Include &include);
    void removeInclude (const Include &include);
    void addInclude (const Include &include);
    void reorderIncludes (const Includes &includes);
    void removeNewLinesBefore (int line, bool isNew);

    CPlusPlus::Scope * scopeAtToken (unsigned token) const;
    int lineAfterFirstComment () const;

    QString file () const;

    const CPlusPlus::Snapshot &snapshot () const;
    CPlusPlus::Document::Ptr cppDocument () const;
    CPlusPlus::TranslationUnit * translationUnit () const;

  private:
    int realLine (int line, bool isNew) const;
    void addToCppDocument (const Include &include);
    QTextCursor cursor (int line, bool isNew = false);

    Core::IDocument *idocument_;
    CPlusPlus::Document::Ptr cppDocument_;
    CPlusPlus::Snapshot snapshot_;
    QTextDocument *textDocument_;
    QStringList includePaths_;
    QString projectPath_;
    QList<int> includeLines_;
    QMap<int, int> lineChanges_;
};

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

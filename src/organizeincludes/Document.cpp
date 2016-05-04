#include "Document.h"

#include <cpptools/cppmodelmanager.h>
#include <cpptools/projectpart.h>

#include <coreplugin/editormanager/editormanager.h>

#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>

#include <utils/qtcassert.h>

#include <QTextBlock>

using namespace Core;
using namespace CppTools;
using namespace CPlusPlus;

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

Document::Document (Core::IDocument *idocument)
  : idocument_ (idocument)
{
  if (!idocument_) {
    return;
  }
  auto model = CppModelManager::instance ();
  snapshot_ = model->snapshot ();
  auto documentFile = idocument_->filePath ().toString ();
  cppDocument_ = snapshot_.preprocessedDocument (idocument_->contents (), documentFile);
  if (!cppDocument_ || !cppDocument_->parse ()) {
    qDebug () << "parse failed";
    return;
  }
  cppDocument_->check ();
}

bool Document::isValid () const
{
  return (idocument_ && cppDocument_);
}

Includes Document::includes () const
{
  Includes includes;
  for (const auto &i: cppDocument_->resolvedIncludes ()) {
    includes << (i);
  }
  for (const auto &i: cppDocument_->unresolvedIncludes ()) {
    includes << (i);
  }
  return includes;
}

void Document::addInclude (const Include &include)
{
  cppDocument_->addIncludeFile (CPlusPlus::Document::Include (
                                  include.include, include.file,
                                  include.line, Client::IncludeLocal));
  if (!snapshot_.contains (include.file)) {
    QFile f (include.file);
    if (f.open (QFile::ReadOnly)) {
      auto resolved = snapshot_.preprocessedDocument (f.readAll (), include.file);
      resolved->parse ();
      resolved->check ();
      snapshot_.insert (resolved);
    }
  }
}

QList<int> Document::includeLines () const
{
  QList<int> lines;
  auto all = cppDocument_->resolvedIncludes () + cppDocument_->unresolvedIncludes ();
  std::transform (all.cbegin (), all.cend (), std::back_inserter (lines),
                  [] (const CPlusPlus::Document::Include &i) -> int {
          return Include (i).isMoc () ? -1 : i.line () - 1;
        });
  lines.removeAll (-1);
  return lines;
}

QList<QString> Document::includePaths () const
{
  QList<QString> result;
  auto model = CppModelManager::instance ();
  auto parts = model->projectPart (cppDocument_->fileName ());
  for (const auto &part: parts) {
    for (const auto &path: part->headerPaths) {
      result << path.path;
    }
  }
  return result;
}

QString Document::projectPath () const
{
  auto model = CppModelManager::instance ();
  auto parts = model->projectPart (cppDocument_->fileName ());
  for (const auto &part: parts) {
    return QFileInfo (part->projectFile).absolutePath ();
  }
  return {};
}

QString Document::file () const
{
  return cppDocument_->fileName ();
}

const Snapshot &Document::snapshot () const
{
  return snapshot_;
}

CPlusPlus::Document::Ptr Document::cppDocument () const
{
  return cppDocument_;
}

TranslationUnit * Document::translationUnit () const
{
  return cppDocument_->translationUnit ();
}

QTextDocument * Document::textDocument () const
{
  auto editor = qobject_cast<TextEditor::BaseTextEditor *>(
    Core::EditorManager::activateEditorForDocument (idocument_));
  if (editor) {
    return editor->textDocument ()->document ();
  }
  return nullptr;
}

Scope * Document::scopeAtToken (unsigned token) const
{
  unsigned line = 0;
  unsigned column = 0;
  cppDocument_->translationUnit ()->getTokenStartPosition (token, &line, &column);
  return cppDocument_->scopeAt (line);
}

int Document::lineAfterFirstComment () const
{
  // copied from includeutils.cpp
  int insertLine = -1;

  auto doc = textDocument ();
  if (!doc) {
    return 0;
  }
  QTextBlock block = doc->firstBlock ();
  while (block.isValid ()) {
    const QString trimmedText = block.text ().trimmed ();

    // Only skip the first comment!
    if (trimmedText.startsWith (QLatin1String ("/*"))) {
      do {
        const int pos = block.text ().indexOf (QLatin1String ("*/"));
        if (pos > -1) {
          insertLine = block.blockNumber () + 2;
          break;
        }
        block = block.next ();
      }
      while (block.isValid ());
      break;
    }
    else if (trimmedText.startsWith (QLatin1String ("//"))) {
      block = block.next ();
      while (block.isValid ()) {
        if (!block.text ().trimmed ().startsWith (QLatin1String ("//"))) {
          insertLine = block.blockNumber () + 1;
          break;
        }
        block = block.next ();
      }
      break;
    }

    if (!trimmedText.isEmpty ()) {
      break;
    }
    block = block.next ();
  }

  return insertLine;
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

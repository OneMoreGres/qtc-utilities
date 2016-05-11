#include "Document.h"

#include <cpptools/cppmodelmanager.h>
#include <cpptools/projectpart.h>
#include <cpptools/includeutils.h>

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
  : idocument_ (idocument), textDocument_ (nullptr)
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

  auto editor = qobject_cast<TextEditor::BaseTextEditor *>(
    Core::EditorManager::activateEditorForDocument (idocument_));
  if (editor) {
    textDocument_ = editor->textDocument ()->document ();
  }

  auto parts = model->projectPart (cppDocument_->fileName ());
  for (const auto &part: parts) {
    for (const auto &path: part->headerPaths) {
      includePaths_ << path.path;
      if (projectPath_.isEmpty ()) {
        projectPath_ = QFileInfo (part->projectFile).absolutePath ();
      }
    }
  }
}

bool Document::isValid () const
{
  return (idocument_ && cppDocument_ && textDocument_);
}

Includes Document::includes () const
{
  Includes includes;
  auto all = cppDocument_->resolvedIncludes () + cppDocument_->unresolvedIncludes ();
  for (const auto &i: all) {
    auto include = Include (i);
    if (include.isMoc ()) {
      continue;
    }
    includes << include;
  }
  return includes;
}

QStringList Document::includePaths () const
{
  return includePaths_;
}

QString Document::projectPath () const
{
  return projectPath_;
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

int Document::lineForInclude (const Include &include) const
{
  return IncludeUtils::LineForNewIncludeDirective (textDocument_,
                                                   cppDocument_) (include.file) - 1;
}

void Document::replaceInclude (const Include &include)
{
  auto c = cursor (include.line, include.isAdded);
  c.select (QTextCursor::LineUnderCursor);
  c.removeSelectedText ();
  c.insertText (include.directive ());

  addToCppDocument (include);
}

void Document::removeInclude (const Include &include)
{
  auto c = cursor (include.line, include.isAdded);
  c.select (QTextCursor::LineUnderCursor);
  c.removeSelectedText ();
  c.deleteChar (); // for eol
  lineChanges_[include.line] -= 1;
}

void Document::addInclude (const Include &include)
{
  auto c = cursor (include.line);
  c.insertText (include.directive () + QLatin1String ("\n"));
  lineChanges_[include.line] += 1;
}

void Document::reorderIncludes (const Includes &includes)
{
  QMap<int, QString> names;
  QSet<int> groupLines;
  auto lastGroup = -1;
  auto line = lineAfterFirstComment () - 1;
  for (const auto &i: includes) {
    names[++line] = i.directive ();
    removeInclude (i);
    removeNewLinesBefore (i.line, true);
    if (i.groupIndex != lastGroup && lastGroup != -1) {
      groupLines.insert (line);
    }
    lastGroup = i.groupIndex;
  }

  lineChanges_.clear ();
  for (auto line: names.keys ()) {
    auto c = cursor (line, true);
    if (groupLines.contains (line)) {
      c.insertText (QLatin1String ("\n"));
      lineChanges_[line] += 1;
    }
    c.insertText (names[line] + QLatin1String ("\n"));
  }
}

void Document::removeNewLinesBefore (int line, bool isNew)
{
  auto c = cursor (line, isNew);
  while (true) {
    c.movePosition (QTextCursor::Up);
    if (c.block ().text ().isEmpty ()) {
      c.deleteChar ();
      lineChanges_[--line] -= 1;
      continue;
    }
    break;
  }
}

int Document::realLine (int line, bool isNew) const
{
  int result = line;
  for (auto i: lineChanges_.keys ()) {
    if (i > line || (isNew && i == line)) {
      break;
    }
    result += lineChanges_[i];
  }
  return result;
}

QTextCursor Document::cursor (int line, bool isNew)
{
  return QTextCursor (textDocument_->findBlockByLineNumber (realLine (line, isNew)));
}

void Document::addToCppDocument (const Include &include)
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

  QTextBlock block = textDocument_->firstBlock ();
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

  return insertLine - 1;
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

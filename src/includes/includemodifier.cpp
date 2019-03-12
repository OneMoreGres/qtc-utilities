#include "includemodifier.h"

#include "includetree.h"

#include <coreplugin/editormanager/editormanager.h>

#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
#include <texteditor/textdocumentlayout.h>
#include <texteditor/codeassist/textdocumentmanipulator.h>

#include <utils/qtcassert.h>

#include <QDebug>

IncludeModifier::IncludeModifier (CPlusPlus::Document::Ptr document) :
  document_ (document) {

  auto editor = qobject_cast<TextEditor::BaseTextEditor *>(
    Core::EditorManager::openEditor (document_->fileName ()));
  if (editor) {
    textDocument_ = editor->textDocument ()->document ();
  }
  unfoldDocument ();
}

void IncludeModifier::queueDuplicatesRemoval () {
  QTC_ASSERT (document_, return );
  QSet<QString> used;
  for (const auto &include: document_->resolvedIncludes ()) {
    if (!used.contains (include.resolvedFileName ())) {
      used.insert (include.resolvedFileName ());
      continue;
    }
    qDebug () << "remove duplicate" << include.line () << include.unresolvedFileName ();
    removeIncludeAt (include.line () - 1);
  }
}

void IncludeModifier::queueUpdates (const IncludeTree &tree) {
  const auto become = tree.includes ();
  for (const auto &include: document_->resolvedIncludes ()) {
    if (!become.contains (include.resolvedFileName ())) {
      qDebug () << "remove include" << include.line () << include.unresolvedFileName ();
      removeIncludeAt (include.line () - 1);
    }
  }
}

void IncludeModifier::executeQueue () {
  std::sort (linesToRemove_.begin (), linesToRemove_.end (), std::greater<int>());
  auto prev = 0;
  for (auto line: linesToRemove_) {
    if (line == prev) {
      continue;
    }
    prev = line;
    auto c = QTextCursor (textDocument_->findBlockByLineNumber (line));
    c.select (QTextCursor::LineUnderCursor);
    c.removeSelectedText ();
    c.deleteChar ();       // for eol
  }
  linesToRemove_.clear ();
}

void IncludeModifier::removeIncludeAt (int line) {
  auto c = QTextCursor (textDocument_->findBlockByLineNumber (line));
  linesToRemove_.append (line);
  while (true) {
    --line;
    c.movePosition (QTextCursor::Up);
    if (!c.block ().text ().isEmpty ()) {
      break;
    }
    linesToRemove_.append (line);
  }
}

void IncludeModifier::removeNewLinesBefore (int line) {
  auto c = QTextCursor (textDocument_->findBlockByLineNumber (line));
  while (true) {
    c.movePosition (QTextCursor::Up);
    if (c.block ().text ().isEmpty ()) {
      c.deleteChar ();
      continue;
    }
    break;
  }
}

void IncludeModifier::unfoldDocument () {
  for (auto i = 0, end = textDocument_->blockCount (); i < end; ++i) {
    auto block = textDocument_->findBlockByNumber (i);
    if (TextEditor::TextDocumentLayout::isFolded (block)) {
      TextEditor::TextDocumentLayout::doFoldOrUnfold (block, true);
    }
  }
}
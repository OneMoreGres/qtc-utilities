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
  qCritical () << "IncludeModifier for document" << document_->fileName ();

  auto editor = qobject_cast<TextEditor::BaseTextEditor *>(
    Core::EditorManager::openEditor (document_->fileName ()));
  if (editor) {
    qCritical () << "IncludeModifier opened editor" << editor->document ()->filePath ();
    textDocument_ = editor->textDocument ()->document ();
  }
  unfoldDocument ();
}

void IncludeModifier::queueDuplicatesRemoval () {
  qCritical () << "queueDuplicatesRemoval";
  QTC_ASSERT (document_, return );
  QSet<QString> used;
  for (const auto &include: document_->resolvedIncludes ()) {
    if (include.line () < 1) {
      continue;
    }
    if (!used.contains (include.resolvedFileName ())) {
      used.insert (include.resolvedFileName ());
      continue;
    }
    qCritical () << "remove duplicate" << include.line () << include.unresolvedFileName ();
    removeIncludeAt (include.line () - 1);
  }
}

void IncludeModifier::queueUpdates (const IncludeTree &tree) {
  const auto become = tree.includes ();
  qCritical () << "queueUpdates. Used" << become;
  for (const auto &include: document_->resolvedIncludes ()) {
    if (include.line () < 1) {
      continue;
    }
    qCritical () << "resolved include of" << document_->fileName ()
                 << include.resolvedFileName () << include.unresolvedFileName ()
                 << include.line ();
    if (!become.contains (include.resolvedFileName ())) {
      qCritical () << "remove include" << include.line () << include.unresolvedFileName ()
                   << include.resolvedFileName ();
      removeIncludeAt (include.line () - 1);
    }
  }
}

void IncludeModifier::executeQueue () {
  qCritical () << "executeQueue";
  std::sort (linesToRemove_.begin (), linesToRemove_.end (), std::greater<int>());
  for (auto line: linesToRemove_) {
    auto c = QTextCursor (textDocument_->findBlockByLineNumber (line));
    c.select (QTextCursor::LineUnderCursor);
    c.removeSelectedText ();
    c.deleteChar ();       // for eol
  }
  linesToRemove_.clear ();
}

void IncludeModifier::removeIncludeAt (int line) {
  linesToRemove_.append (line);
  if (isGroupRemoved (line)) {
    removeTillNextGroup (line);
  }
}

void IncludeModifier::removeTillNextGroup (int line) {
  auto c = QTextCursor (textDocument_->findBlockByLineNumber (line));
  auto inBlock = true;
  while (true) {
    ++line;
    if (!c.movePosition (QTextCursor::Down)) {
      break;
    }
    if (!c.block ().text ().isEmpty ()) {
      if (inBlock) {
        continue;
      }
      break;
    }
    inBlock = false;
    linesToRemove_.append (line);
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

bool IncludeModifier::isGroupRemoved (int line) const {
  auto c = QTextCursor (textDocument_->findBlockByLineNumber (line));

  while (true) {
    ++line;
    if (!c.movePosition (QTextCursor::Down)) {
      break;
    }
    if (c.block ().text ().isEmpty ()) {
      break;
    }
    if (linesToRemove_.contains (line)) {
      continue;
    }
    return false;
  }

  c = QTextCursor (textDocument_->findBlockByLineNumber (line));
  while (true) {
    --line;
    if (!c.movePosition (QTextCursor::Up)) {
      break;
    }
    if (c.block ().text ().isEmpty ()) {
      break;
    }
    if (linesToRemove_.contains (line)) {
      continue;
    }
    return false;
  }

  return true;
}

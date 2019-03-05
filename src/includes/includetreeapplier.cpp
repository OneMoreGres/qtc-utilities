#include "includetreeapplier.h"

#include "includetree.h"

#include <coreplugin/editormanager/editormanager.h>

#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
#include <texteditor/textdocumentlayout.h>
#include <texteditor/codeassist/textdocumentmanipulator.h>

#include <utils/qtcassert.h>

#include <QDebug>

IncludeTreeApplier::IncludeTreeApplier (CPlusPlus::Document::Ptr document) :
  document_ (document) {

  auto editor = qobject_cast<TextEditor::BaseTextEditor *>(
    Core::EditorManager::openEditor (document_->fileName ()));
  if (editor) {
    textDocument_ = editor->textDocument ()->document ();
  }
  unfoldDocument ();
}

void IncludeTreeApplier::removeDuplicates () {
  QTC_ASSERT (document_, return );
  QSet<QString> used;
  for (const auto &include: document_->resolvedIncludes ()) {
    if (!used.contains (include.resolvedFileName ())) {
      used.insert (include.resolvedFileName ());
      continue;
    }
    qDebug () << "remove duplicate" << include.line () << include.unresolvedFileName ();
    removeIncludeAt (include.line ());
  }
}

void IncludeTreeApplier::apply (const IncludeTree &tree) {
  //    TextEditor::TextDocumentManipulator manipulator(
  //Core::EditorManager::openEditor (document_->fileName ()))
  QTC_ASSERT (document_, return );
  const auto become = tree.includes ();
  for (const auto &include: document_->resolvedIncludes ()) {
    if (!become.contains (include.resolvedFileName ())) {
      qDebug () << "remove include" << include.line () << include.unresolvedFileName ();
      removeIncludeAt (include.line ());
    }
  }
  //  for (const auto &include: document_->unresolvedIncludes ()) {
  //    if (!become.contains (include.resolvedFileName ())) {
  //      removeIncludeAt (include.line ());
  //    }
  //  }
  //  const auto existing = document_->includedFiles ();
}

void IncludeTreeApplier::removeIncludeAt (int line) {
  removeNewLinesBefore (line, false);
  auto c = cursor (line, false);
  c.select (QTextCursor::LineUnderCursor);
  c.removeSelectedText ();
  c.deleteChar ();   // for eol
  lineChanges_[line] -= 1;
}

void IncludeTreeApplier::removeNewLinesBefore (int line, bool isNew) {
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

int IncludeTreeApplier::realLine (int line, bool isNew) const {
  auto result = line - 1; // to start from 0
  for (auto i: lineChanges_.keys ()) {
    if (i > line || (isNew && i == line)) {
      break;
    }
    result += lineChanges_[i];
  }
  return result;
}

QTextCursor IncludeTreeApplier::cursor (int line, bool isNew) {
  const auto realLine = this->realLine (line, isNew);
  qDebug () << "real" << realLine;
  return QTextCursor (textDocument_->findBlockByLineNumber (realLine));
}

int IncludeTreeApplier::lineAfterFirstComment () const {
  // copied from includeutils.cpp
  auto insertLine = 1;

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

void IncludeTreeApplier::unfoldDocument () {
  for (auto i = 0, end = textDocument_->blockCount (); i < end; ++i) {
    auto block = textDocument_->findBlockByNumber (i);
    if (TextEditor::TextDocumentLayout::isFolded (block)) {
      TextEditor::TextDocumentLayout::doFoldOrUnfold (block, true);
    }
  }
}

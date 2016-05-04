#include "IncludesOrganizer.h"
#include "IncludesSettings.h"
#include "IncludesConstants.h"
#include "Include.h"
#include "IncludesOptionsPage.h"
#include "IncludesExtractor.h"
#include "IncludeMap.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>

#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>

#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/iplugin.h>

#include <cpptools/cppmodelmanager.h>
#include <cpptools/cpplocatorfilter.h>
#include <cpptools/projectpart.h>

#include <QMenu>
#include <QTextBlock>
#include <QDir>
#include <QComboBox>
#include <QBoxLayout>

using namespace Core;
using namespace CppTools;
using namespace CPlusPlus;

namespace {
// copied from includeutils.cpp
int lineAfterFirstComment (const QTextDocument *textDocument)
{
  int insertLine = -1;

  QTextBlock block = textDocument->firstBlock ();
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
}


namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {



using StringFiles = QList<QString>;



StringFiles getFileIncludePaths (const QString &file)
{
  StringFiles result;
  auto model = CppModelManager::instance ();
  auto parts = model->projectPart (file);
  for (const auto &part: parts) {
    for (const auto &path: part->headerPaths) {
      result << path.path;
    }
  }
  return result;
}

Includes getDocumentIncludes (Document::Ptr document)
{
  Includes includes;
  for (const auto &i: document->resolvedIncludes ()) {
    includes << (i);
  }
  for (const auto &i: document->unresolvedIncludes ()) {
    includes << (i);
  }
  return includes;
}


void addIncludeToDocument (const Include &include, Document::Ptr document,
                           Snapshot &snapshot)
{
  document->addIncludeFile (Document::Include (include.include, include.file,
                                               include.line, Client::IncludeLocal));
  if (!snapshot.contains (include.file)) {
    QFile f (include.file);
    if (f.open (QFile::ReadOnly)) {
      auto resolved = snapshot.preprocessedDocument (f.readAll (), include.file);
      resolved->parse ();
      resolved->check ();
      snapshot.insert (resolved);
    }
  }
}






QString resolveInclude (const QString &nameOnly, const QString &asIncluded,
                        const QString &path)
{
  QDir dir (path);
  if (dir.exists (nameOnly)) {     // file path starts below include path
    return dir.absoluteFilePath (nameOnly);
  }
  for (const auto &subdir: dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot)) {
    auto resolved = resolveInclude (nameOnly, asIncluded, dir.filePath (subdir));
    if (!resolved.isEmpty ()
        && resolved.endsWith (QDir::separator () + asIncluded)) {
      return resolved;
    }
  }
  return {};
}

void resolveIncludes (Includes &includes, const StringFiles &includePaths)
{
  for (auto &i: includes) {
    if (!i.file.isEmpty ()) {
      continue;
    }
    auto nameOnly = QFileInfo (i.include).fileName ();
    for (const auto &path: includePaths) {
      i.file = resolveInclude (nameOnly, i.include, path);
      if (!i.file.isEmpty ()) {
        i.isResolvedNow = true;
        break;
      }
    }
  }
}




void renameIncludes (Includes &includes, const StringFiles &includePaths,
                     const QString &localPath)
{
  for (auto &i: includes) {
    i.isLocal = i.file.startsWith (localPath);
    if (!i.include.isEmpty () && !i.isResolvedNow) {
      continue;
    }
    auto file = i.file;
    auto best = std::max_element (includePaths.cbegin (), includePaths.cend (),
                                  [&file](const QString &l, const QString &r) {
            if (!file.startsWith (l)) {
              return true;
            }
            if (!file.startsWith (r)) {
              return false;
            }
            return l.size () < r.size ();
          });

    i.include = file.startsWith (*best)
                ? i.file.mid (best->size () + 1)
                : QFileInfo (i.file).fileName ();
  }
}



void sortIncludes (Includes &includes, Order order,
                   const QString &localPath)
{
#ifdef Q_OS_LINUX
  QString systemPath = QStringLiteral ("/usr/");
#else
  QString systemPath = QStringLiteral ("C:/Program Files");
#endif
  switch (order) {
    case GeneralFirst:
    case SpecificFirst:
      std::sort (includes.begin (), includes.end (),
                 [&localPath, &systemPath](const Include &l, const Include &r) {
            if (l.file.startsWith (localPath)) {
              if (r.file.startsWith (localPath)) {
                return l.file > r.file;
              }
              return true;
            }
            if (l.file.startsWith (systemPath)) {
              if (r.file.startsWith (systemPath)) {
                return l.file > r.file;
              }
              return false;
            }
            return l.file > r.file;
          });

      if (order == GeneralFirst) {
        std::reverse (includes.begin (), includes.end ());
      }
      break;

    case KeepCurrent:
      break;

    case Alphabetical:
      std::sort (includes.begin (), includes.end (),
                 [](const Include &l, const Include &r) {
            return l.file < r.file;
          });
      break;

    default:
      qCritical () << "Unhandled sort switch";
      break;
  }
}


void writeIncludes (QTextDocument *textDocument, Document::Ptr cppDocument,
                    const Includes &includes)
{
  QList<int> oldLines;
  auto old = cppDocument->resolvedIncludes () + cppDocument->unresolvedIncludes ();
  std::transform (old.cbegin (), old.cend (), std::back_inserter (oldLines),
                  [] (const Document::Include &i) -> int {
          return Include (i).isMoc () ? -1 : i.line () - 1;
        });

  std::sort (oldLines.begin (), oldLines.end (), std::greater<int>());

  std::for_each (oldLines.cbegin (), oldLines.cend (), [textDocument] (int i) {
          if (i > -1) {
            QTextCursor c (textDocument->findBlockByLineNumber (i));
            c.select (QTextCursor::BlockUnderCursor);
            c.removeSelectedText ();
          }
        });

  if (includes.isEmpty ()) {
    return;
  }

  QString text;
  QString last;
  std::for_each (includes.cbegin (), includes.cend (),
                 [&text, &last] (const Include &i) {
          if (i.isMoc ()) {
            return;
          }
          if (!last.isEmpty ()) {
            auto lastPath = QFileInfo (last).absolutePath ();
            auto path = QFileInfo (i.file).absolutePath ();
            if (!path.startsWith (lastPath) && !lastPath.startsWith (path)) {
              text += QStringLiteral ("\n");
            }
          }
          last = i.file;
          text += i.directive ();
        });

  QTextCursor c (textDocument);
  int linesToMove = lineAfterFirstComment (textDocument) - 1;
  if (linesToMove > 0) {
    c.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, linesToMove);
  }
  c.insertText (text);
}




IncludesOrganizer::IncludesOrganizer (ExtensionSystem::IPlugin *plugin) :
  options_ (new IncludesOptionsPage)
{
  registerActions ();
  plugin->addAutoReleasedObject (options_.data ());
}


void IncludesOrganizer::registerActions ()
{
  auto menu = ActionManager::createMenu (MENU_ID);
  menu->menu ()->setTitle (tr ("OrganizeIncludes"));
  ActionManager::actionContainer (Core::Constants::M_TOOLS)->addMenu (menu);

  {
    auto action = new QAction (tr ("Organize includes"), this);
    connect (action, SIGNAL (triggered ()), this, SLOT (organize ()));
    auto command = ActionManager::registerAction (action, ACTION_ORGANIZE_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Ctrl+Shift+A")));
    menu->addAction (command);
  }
}


void IncludesOrganizer::organize (int actions) const
{
  auto idocument = EditorManager::currentDocument ();
  auto editor = qobject_cast<TextEditor::BaseTextEditor *>(EditorManager::currentEditor ());
  if (!idocument || !editor || !options_) {
    return;
  }
  auto model = CppModelManager::instance ();
  auto snapshot = model->snapshot ();
  auto documentFile = idocument->filePath ().toString ();
  auto document = snapshot.preprocessedDocument (idocument->contents (), documentFile);
  if (!document || !document->parse ()) {
    qDebug () << "parse failed";
    return;
  }
  document->check ();

  Includes includes = getDocumentIncludes (document);
  qDebug () << "doc includes" << includes;

  auto includePaths = getFileIncludePaths (documentFile);
  if (includePaths.isEmpty ()) {
    qDebug () << "empty include paths";
    return;
  }

  if (actions | Resolve) {
    resolveIncludes (includes, includePaths);
    qDebug () << "resolved includes" << includes;
    for (const auto &i: includes) {
      if (i.isResolvedNow) {
        addIncludeToDocument (i, document, snapshot);
      }
    }
  }

  Includes usedIncludes = IncludesExtractor (document, snapshot) ();
  qDebug () << "usedIncludes" << usedIncludes;
  usedIncludes.removeAll (documentFile);

  IncludeMap map (snapshot, includes, usedIncludes);
  const auto &settings = options_->settings ();
  map.organize (settings.policy);

  qDebug () << "left includers/includes" <<  map.includers () << map.includes ();

  if (actions | Remove) {
    auto unused = map.includers ();
    for (const auto &i: unused) {
      includes.removeAll (i);
    }
  }

  if (actions | Add) {
    auto added = map.includes ();
    includes += added;
  }

  auto localPath = *includePaths.begin ();     // TODO ensure local is first
  renameIncludes (includes, includePaths, localPath);

  if (actions | Sort) {
    sortIncludes (includes, settings.order, localPath);
  }

  auto textDocument = editor->textDocument ()->document ();
  writeIncludes (textDocument, document, includes);
}


void IncludesOrganizer::organize ()
{
  organize (AllActions);
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcCodeUtils

//#include "IncludesOrganizer.moc"

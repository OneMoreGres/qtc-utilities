#include "IncludesOrganizer.h"
#include "IncludesConstants.h"
#include "Include.h"
#include "IncludesOptionsPage.h"
#include "IncludesExtractor.h"
#include "IncludeMap.h"
#include "Document.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>

#include <extensionsystem/iplugin.h>

#include <QMenu>
#include <QTextBlock>
#include <QDir>

using namespace Core;


namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

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

void resolveIncludes (Includes &includes, const QList<QString> &includePaths)
{
  for (auto &i: includes) {
    if (!i.file.isEmpty ()) {
      continue;
    }
    auto nameOnly = QFileInfo (i.include).fileName ();
    for (const auto &path: includePaths) {
      i.file = resolveInclude (nameOnly, i.include, path);
      if (!i.file.isEmpty ()) {
        i.isJustResolved = true;
        break;
      }
    }
  }
}




void renameIncludes (Includes &includes, const QList<QString> &includePaths,
                     const QString &projectPath)
{
  for (auto &i: includes) {
    i.isLocal = i.file.startsWith (projectPath);

    if (!(i.include.isEmpty () || i.isJustResolved)) {
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



void sortIncludes (Includes &includes, Order order, const QString &projectPath)
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
                 [&projectPath, &systemPath](const Include &l, const Include &r) {
            if (l.file.startsWith (projectPath)) {
              if (r.file.startsWith (projectPath)) {
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



void writeIncludes (Document &document, const Includes &includes)
{
  auto textDocument = document.textDocument ();
  auto oldLines = document.includeLines ();
  std::sort (oldLines.begin (), oldLines.end (), std::greater<int>());

  std::for_each (oldLines.cbegin (), oldLines.cend (), [textDocument] (int i) {
          QTextCursor c (textDocument->findBlockByLineNumber (i));
          c.select (QTextCursor::BlockUnderCursor);
          c.removeSelectedText ();
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
  int linesToMove = document.lineAfterFirstComment () - 1;
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
  Document document (EditorManager::currentDocument ());
  if (!options_ || !document.isValid ()) {
    return;
  }

  auto includePaths = document.includePaths ();
  if (includePaths.isEmpty ()) {
    qDebug () << "empty include paths";
    return;
  }

  Includes includes = document.includes ();
  qDebug () << "doc includes" << includes;

  if (actions | Resolve) {
    resolveIncludes (includes, includePaths);
    qDebug () << "resolved includes" << includes;
    for (const auto &i: includes) {
      if (i.isJustResolved) {
        document.addInclude (i);
      }
    }
  }

  Includes usedIncludes = IncludesExtractor (document) ();
  qDebug () << "usedIncludes" << usedIncludes;

  const auto &settings = options_->settings ();
  IncludeMap map (document.snapshot (), includes, usedIncludes);
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

  auto projectPath = document.projectPath ();
  renameIncludes (includes, includePaths, projectPath);

  if (actions | Sort) {
    sortIncludes (includes, settings.order, projectPath);
  }

  writeIncludes (document, includes);
}


void IncludesOrganizer::organize ()
{
  organize (AllActions);
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcCodeUtils

//#include "IncludesOrganizer.moc"

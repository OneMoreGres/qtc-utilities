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
#include <QDir>

using namespace Core;


namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

void renamePrivateInclude (Include &include)
{
  if (include.file.contains (QDir::separator () + QLatin1String ("Qt"))) {
    QFileInfo info (include.file);
    auto dir = info.absoluteDir ();
    auto files = dir.entryList (QStringList () << info.baseName (), QDir::Files);
    if (files.size () == 1) {
      include.file = dir.absoluteFilePath (files.first ());
    }
  }
}



bool updateInclude (Include &i, const Document &document)
{
  auto isLocal = i.file.startsWith (document.projectPath ());
  auto changed = (i.isLocal != isLocal);
  i.isLocal = isLocal;

  if (i.line < 0) {
    changed = true;
    i.line = document.lineForInclude (i);
    i.isAdded = true;
  }

  if (!i.include.isEmpty ()) {
    return changed;
  }
  auto includePaths = document.includePaths ();
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
              ? file.mid (best->size () + 1)
              : QFileInfo (file).fileName ();
  return true;
}



bool resolveInclude (Include &include, const QString &path)
{
  auto nameOnly = QFileInfo (include.include).fileName ();
  QDir dir (path);
  if (dir.exists (nameOnly)) {
    auto file = dir.absoluteFilePath (nameOnly);
    if (file.endsWith (QDir::separator () + include.include)) {
      include.file = file;
      include.include.clear ();
      return true;
    }
  }
  for (const auto &subdir: dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot)) {
    if (resolveInclude (include, dir.filePath (subdir))) {
      return true;
    }
  }
  return false;
}

void resolveIncludes (Includes &includes, Document &document)
{
  for (auto &i: includes) {
    if (!i.file.isEmpty ()) {
      continue;
    }
    for (const auto &path: document.includePaths ()) {
      if (resolveInclude (i, path)) {
        updateInclude (i, document);
        document.replaceInclude (i);
        break;
      }
    }
  }
}



void sortIncludes (Includes &includes, Order order, Document &document)
{
  auto projectPath = document.projectPath ();
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

  if (order == Alphabetical || order == KeepCurrent) {
    return;
  }

  QString last;
  auto groupIndex = 0;
  for (auto &i: includes) {
    auto path = QFileInfo (i.file).absolutePath ();
    if (!last.startsWith (path) && !path.startsWith (last) && !last.isEmpty ()) {
      ++groupIndex;
    }
    i.groupIndex = groupIndex;
    last = path;
  }
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
  menu->menu ()->setTitle (tr ("Includes"));
  ActionManager::actionContainer (Core::Constants::M_TOOLS)->addMenu (menu);

  {
    auto action = new QAction (tr ("Organize includes"), this);
    connect (action, &QAction::triggered, this, &IncludesOrganizer::organize);
    auto command = ActionManager::registerAction (action, ACTION_ORGANIZE_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+O")));
    menu->addAction (command);
  }
  {
    auto action = new QAction (tr ("Sort includes"), this);
    connect (action, &QAction::triggered, this, &IncludesOrganizer::sort);
    auto command = ActionManager::registerAction (action, ACTION_SORT_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+S")));
    menu->addAction (command);
  }
  {
    auto action = new QAction (tr ("Add missing includes"), this);
    connect (action, &QAction::triggered, this, &IncludesOrganizer::add);
    auto command = ActionManager::registerAction (action, ACTION_ADD_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+M")));
    menu->addAction (command);
  }
  {
    auto action = new QAction (tr ("Remove unused includes"), this);
    connect (action, &QAction::triggered, this, &IncludesOrganizer::remove);
    auto command = ActionManager::registerAction (action, ACTION_REMOVE_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+U")));
    menu->addAction (command);
  }
  {
    auto action = new QAction (tr ("Resolve includes"), this);
    connect (action, &QAction::triggered, this, &IncludesOrganizer::resolve);
    auto command = ActionManager::registerAction (action, ACTION_RESOLVE_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+R")));
    menu->addAction (command);
  }
  {
    auto action = new QAction (tr ("Rename includes"), this);
    connect (action, &QAction::triggered, this, &IncludesOrganizer::rename);
    auto command = ActionManager::registerAction (action, ACTION_RENAME_INCLUDES);
    command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+N")));
    menu->addAction (command);
  }
}


void IncludesOrganizer::applyActions (int actions) const
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

  if (actions & Resolve) {
    resolveIncludes (includes, document);
    qDebug () << "resolved includes" << includes;
  }

  Includes usedIncludes = IncludesExtractor (document) ();
  qDebug () << "usedIncludes" << usedIncludes;

  std::for_each (includes.begin (), includes.end (), &renamePrivateInclude);
  std::for_each (usedIncludes.begin (), usedIncludes.end (), &renamePrivateInclude);

  const auto &settings = options_->settings ();
  IncludeMap map (document.snapshot (), includes, usedIncludes);
  map.organize (settings.policy);
  qDebug () << "left includers/includes" << map.includers () << map.includes ();

  if (actions & Remove) {
    auto unused = map.includers ();
    for (const auto &i: unused) {
      includes.removeAll (i);
      document.removeInclude (i);
    }
  }

  if (actions & Add) {
    auto added = map.includes ();
    for (auto &i: added) {
      updateInclude (i, document);
      document.addInclude (i);
      includes << i;
    }
  }

  if (actions & Rename) {
    for (auto &i: includes) {
      if (updateInclude (i, document)) {
        document.replaceInclude (i);
      }
    }
  }

  if (actions & Sort) {
    sortIncludes (includes, settings.order, document);
    document.reorderIncludes (includes);
  }
}


void IncludesOrganizer::organize ()
{
  applyActions (options_->settings ().organizeActions);
}

void IncludesOrganizer::sort ()
{
  applyActions (Sort);
}

void IncludesOrganizer::add ()
{
  applyActions (Add);
}

void IncludesOrganizer::remove ()
{
  applyActions (Remove);
}

void IncludesOrganizer::resolve ()
{
  applyActions (Resolve);
}

void IncludesOrganizer::rename ()
{
  applyActions (Rename);
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcCodeUtils

//#include "IncludesOrganizer.moc"

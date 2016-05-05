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

#include <cpptools/includeutils.h>

#include <QMenu>
#include <QTextBlock>
#include <QDir>

using namespace Core;


namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {


bool updateInclude (Include &i, const Document &document)
{
  auto isLocal = i.file.startsWith (document.projectPath ());
  auto changed = (i.isLocal != isLocal);
  i.isLocal = isLocal;

  if (i.line < 0) {
    changed = true;
    i.line = CppTools::IncludeUtils::LineForNewIncludeDirective (
      document.textDocument (), document.cppDocument ()) (i.file) - 1;
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
              ? i.file.mid (best->size () + 1)
              : QFileInfo (i.file).fileName ();
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
    if (!last.isEmpty ()) {
      auto lastPath = QFileInfo (last).absolutePath ();
      auto path = QFileInfo (i.file).absolutePath ();
      if (!path.startsWith (lastPath) && !lastPath.startsWith (path)) {
        i.groupIndex = ++groupIndex;
      }
    }
    last = i.file;
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
    resolveIncludes (includes, document);
    qDebug () << "resolved includes" << includes;
  }

  Includes usedIncludes = IncludesExtractor (document) ();
  qDebug () << "usedIncludes" << usedIncludes;

  const auto &settings = options_->settings ();
  IncludeMap map (document.snapshot (), includes, usedIncludes);
  map.organize (settings.policy);
  qDebug () << "left includers/includes" << map.includers () << map.includes ();

  if (actions | Remove) {
    auto unused = map.includers ();
    for (const auto &i: unused) {
      includes.removeAll (i);
      document.removeInclude (i);
    }
  }

  if (actions | Add) {
    auto added = map.includes ();
    for (auto &i: added) {
      updateInclude (i, document);
      document.addInclude (i);
      includes << i;
    }
  }

  if (actions | Rename) {
    for (auto &i: includes) {
      if (updateInclude (i, document)) {
        document.replaceInclude (i);
      }
    }
  }

  if (actions | Sort) {
    sortIncludes (includes, settings.order, document);
    document.reorderIncludes (includes);
  }
}


void IncludesOrganizer::organize ()
{
  organize (AllActions);
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcCodeUtils

//#include "IncludesOrganizer.moc"

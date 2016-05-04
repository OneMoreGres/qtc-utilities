#include "AutoCheckEvents.h"
#include "CheckTypes.h"

#include <coreplugin/editormanager/editormanager.h>

#include <projectexplorer/project.h>

#include <projectexplorer/projecttree.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/session.h>

#include <utils/qtcassert.h>

using namespace ProjectExplorer;
using namespace Core;


namespace QtcUtilities {
namespace Internal {
namespace ClangTools {

AutoCheckEvents::AutoCheckEvents (QObject *parent) : QObject (parent)
{
  connect (SessionManager::instance (),
           SIGNAL (startupProjectChanged (ProjectExplorer::Project *)),
           SLOT (handleStartupProjectChange (ProjectExplorer::Project *)));

  // Check on build
  connect (BuildManager::instance (),
           SIGNAL (buildStateChanged (ProjectExplorer::Project *)),
           SLOT (handleBuildStateChange (ProjectExplorer::Project *)));

  // Open documents auto check.
  connect (DocumentModel::model (),
           SIGNAL (dataChanged (const QModelIndex &, const QModelIndex &, const QVector<int> &)),
           SLOT (handleDocumentsChange (const QModelIndex &, const QModelIndex &, const QVector<int> &)));
  connect (DocumentModel::model (),
           SIGNAL (rowsAboutToBeRemoved (const QModelIndex &, int, int)),
           SLOT (handleDocumentsClose (const QModelIndex &, int, int)));
}

void AutoCheckEvents::checkCurrentNode ()
{
  if (auto node = ProjectTree::currentNode ()) {
    emitCheckFiles (getFiles (node), CheckManual);
  }
}

void AutoCheckEvents::handleStartupProjectChange (ProjectExplorer::Project *project)
{
  if (!activeProject_.isNull ()) {
    disconnect (activeProject_.data (), SIGNAL (fileListChanged ()),
                this, SLOT (handleProjectFileListChanged ()));
  }
  activeProject_ = project;
  handleProjectFileListChanged ();
  if (!project) {
    return;
  }
  connect (project, SIGNAL (fileListChanged ()), SLOT (handleProjectFileListChanged ()));

  checkActiveProject (CheckOnProjectSwitch);
}

void AutoCheckEvents::handleDocumentsChange (const QModelIndex &topLeft,
                                             const QModelIndex &bottomRight,
                                             const QVector<int> &)
{
  emitCheckFiles (getDocuments (topLeft.row (), bottomRight.row (), false), CheckOnFileChange);
}

void AutoCheckEvents::handleDocumentsClose (const QModelIndex &, int start, int end)
{
  emitCheckFiles (getDocuments (start, end, true), CheckOnFileChange);   // Documents were modified before remove.
}

void AutoCheckEvents::handleBuildStateChange (ProjectExplorer::Project *project)
{
  if (!project || project != activeProject_.data ()) {
    return;
  }
  if (!BuildManager::isBuilding (activeProject_.data ())) { // Finished building.
    checkActiveProject (CheckOnProjectBuild);
  }
}

void AutoCheckEvents::handleProjectFileListChanged ()
{
  if (activeProject_.isNull ()) {
    projectFiles_.clear ();
    return;
  }

  auto oldFiles = projectFiles_.toSet ();
  updateProjectFiles ();
  auto addedFiles = projectFiles_.toSet ().subtract (oldFiles);

  emitCheckFiles (addedFiles.toList (), CheckOnFileAdd);
}

QStringList AutoCheckEvents::getDocuments (int beginRow, int endRow, bool isModified) const
{
  QStringList files;
  for (int row = beginRow; row <= endRow; ++row) {
    if (auto entry = DocumentModel::entryAtRow (row)) {
      if (auto document = entry->document) {
        auto file = document->filePath ().toString ();
        if (projectFiles_.contains (file) && document->isModified () == isModified) {
          files << file;
        }
      }
    }
  }
  return files;
}

QStringList AutoCheckEvents::getFiles (const Node *node) const
{
  QStringList files;
  switch (node->nodeType ()) {
    case FileNodeType:
      {
        const auto file = static_cast<const FileNode *>(node);
        if (file->isGenerated ()) {
          break;
        }
        files << file->filePath ().toString ();
      }
      break;

    case ProjectNodeType:
    case FolderNodeType:
    case VirtualFolderNodeType:
      {
        const FolderNode *folder = static_cast<const FolderNode *> (node);
        for (const auto subfolder: folder->subFolderNodes ()) {
          files += getFiles (subfolder);
        }
        for (const auto file: folder->fileNodes ()) {
          files += getFiles (file);
        }
      }
      break;

    default:
      break;
  }
  return files;
}

void AutoCheckEvents::updateProjectFiles ()
{
  projectFiles_.clear ();
  if (activeProject_) {
    projectFiles_ = getFiles (activeProject_->rootProjectNode ());
  }
}

void AutoCheckEvents::checkActiveProject (int checkType)
{
  if (activeProject_) {
    emitCheckFiles (getFiles (activeProject_->rootProjectNode ()), checkType);
  }
}

void AutoCheckEvents::emitCheckFiles (const QStringList &files, int checkType)
{
  if (!files.isEmpty ()) {
    emit check (files, checkType);
  }
}

} // namespace ClangTools
} // namespace Internal
} // namespace QtcUtilities

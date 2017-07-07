#pragma once

#include <QPointer>

namespace ProjectExplorer {
  class Project;
  class Node;
}

namespace QtcUtilities {
  namespace Internal {
    namespace ClangTools {

      //! Emits signals in situations suitable for automatic analysis.
      class AutoCheckEvents : public QObject {
        Q_OBJECT

        public:
          explicit AutoCheckEvents (QObject *parent = nullptr);

        signals:
          void check (const QStringList &files, int checkType);

        public slots:
          void checkCurrentNode ();

        private slots:
          //! Handle change of Session's startup project.
          void handleStartupProjectChange (ProjectExplorer::Project *project);
          //! Check changed documents if it belongs to active project and not modified.
          void handleDocumentsChange (const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                      const QVector<int> &);
          //! Check closing documents if it belongs to active project and was modified.
          void handleDocumentsClose (const QModelIndex &, int start, int end);
          //! Check built project if it is active and plugin got corresponding settings.
          void handleBuildStateChange (ProjectExplorer::Project *project);
          //! Handle project's fileListChanged signal.
          void handleProjectFileListChanged ();

        private:
          QStringList getDocuments (int beginRow, int endRow, bool isModified) const;
          QStringList getFiles (const ProjectExplorer::Node *node) const;
          void updateProjectFiles ();
          void checkActiveProject (int checkType);
          void emitCheckFiles (const QStringList &files, int checkType);

          QStringList projectFiles_;

          QPointer<ProjectExplorer::Project> activeProject_;
      };

    } // namespace ClangTools
  } // namespace Internal
} // namespace QtcUtilities

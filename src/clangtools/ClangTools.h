#pragma once

#include <QPointer>

namespace ExtensionSystem {
  class IPlugin;
}

namespace QtcUtilities {
  namespace Internal {
    namespace ClangTools {

      class AutoCheckEvents;
      class ToolRunner;
      class ToolOptionsPage;

      class ClangTools : public QObject {
        Q_OBJECT

        public:
          explicit ClangTools (ExtensionSystem::IPlugin *plugin);

        private slots:
          void updateSettings ();

        private:
          void registerActions ();

          using RunnerPtr = QSharedPointer<ToolRunner>;
          using Runners = QList<RunnerPtr>;

          AutoCheckEvents *checkEvents_;
          Runners runners_;
          QPointer<ToolOptionsPage> options_;
      };

    } // namespace ClangTools
  } // namespace Internal
} // namespace QtcUtilities

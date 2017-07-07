#pragma once

#include <QPointer>

namespace ExtensionSystem {
  class IPlugin;
}

namespace QtcUtilities {
  namespace Internal {

    namespace ClangTools {
      class AutoCheckEvents;
    }

    namespace Oclint {

      class OclintRunner;
      class OclintOptionsPage;

      class Oclint : public QObject {
        Q_OBJECT

        public:
          explicit Oclint (ExtensionSystem::IPlugin *plugin);

        private slots:
          void updateSettings ();

        private:
          void registerActions ();

          using RunnerPtr = QSharedPointer<OclintRunner>;

          ClangTools::AutoCheckEvents *checkEvents_;
          RunnerPtr runner_;
          QPointer<OclintOptionsPage> options_;
      };

    } // namespace Oclint
  } // namespace Internal
} // namespace QtcUtilities

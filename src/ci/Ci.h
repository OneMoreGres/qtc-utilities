#pragma once

#include <QObject>

namespace ExtensionSystem {
  class IPlugin;
}

namespace QtcUtilities {
  namespace Internal {
    namespace Ci {

      class Ci : public QObject {
        Q_OBJECT

        public:
          explicit Ci (ExtensionSystem::IPlugin *plugin);
          ~Ci () override;
      };

    } // namespace Ci
  } // namespace Internal
} // namespace QtcUtilities

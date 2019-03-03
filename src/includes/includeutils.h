#pragma once

#include <QObject>

namespace ExtensionSystem {
  class IPlugin;
}

namespace QtcUtilities {
  namespace Internal {
    namespace IncludeUtils {

      class IncludeUtils : public QObject {
        public:
          explicit IncludeUtils (ExtensionSystem::IPlugin *plugin);

        private:
          void organize ();
      };

    }     // namespace IncludeUtils
  }   // namespace Internal
} // namespace QtcUtilities

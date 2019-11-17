#pragma once

#include <QObject>

namespace ExtensionSystem {
  class IPlugin;
}
namespace Core {
    class IEditor;
}

namespace QtcUtilities {
  namespace Internal {
    namespace ScrollBarsColorizer {

      class ScrollBarsColorizer : public QObject {
        public:
          explicit ScrollBarsColorizer (ExtensionSystem::IPlugin *plugin);

        private:
          void adjustColors (Core::IEditor* editor);
      };

    }     // namespace ScrollBarsColorizer
  }       // namespace Internal
} // namespace QtcUtilities

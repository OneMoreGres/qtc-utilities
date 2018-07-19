#include "Ci.h"
#include "Pane.h"

#include <extensionsystem/iplugin.h>

namespace QtcUtilities {
  namespace Internal {
    namespace Ci {

      Ci::Ci (ExtensionSystem::IPlugin *plugin) {
        new Pane (this);
      }

      Ci::~Ci () {

      }

    } // namespace Ci
  } // namespace Internal
} // namespace QtcUtilities

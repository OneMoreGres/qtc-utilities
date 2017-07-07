#pragma once

#include "PluginGlobal.h"

#include <extensionsystem/iplugin.h>

namespace QtcUtilities {
  namespace Internal {

    class QtcUtilitiesPlugin : public ExtensionSystem::IPlugin {
      Q_PLUGIN_METADATA (IID "org.qt-project.Qt.QtCreatorPlugin" FILE "QtcUtilities.json")
      Q_OBJECT

      public:
        QtcUtilitiesPlugin ();
        ~QtcUtilitiesPlugin ();

        bool initialize (const QStringList &arguments, QString *errorString);
        void extensionsInitialized ();
        ShutdownFlag aboutToShutdown ();

      private:
        void initTranslation ();
    };

  } // namespace Internal
} // namespace QtcUtilities

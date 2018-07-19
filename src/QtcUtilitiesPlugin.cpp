#include "QtcUtilitiesPlugin.h"
#include "organizeincludes/IncludesOrganizer.h"
#include "codediscover/CodeDiscover.h"
#include "ci/Ci.h"

#include <coreplugin/icore.h>

#include <QTranslator>
#include <QApplication>

using namespace Core;

namespace QtcUtilities {
  namespace Internal {

    QtcUtilitiesPlugin::QtcUtilitiesPlugin () :
      IPlugin () {
      // Create your members
    }

    QtcUtilitiesPlugin::~QtcUtilitiesPlugin () {
      // Unregister objects from the plugin manager's object pool
      // Delete members
    }

    bool QtcUtilitiesPlugin::initialize (const QStringList & /*arguments*/, QString */*errorString*/) {
      // Register objects in the plugin manager's object pool
      // Load settings
      // Add actions to menus
      // Connect to other plugins' signals
      // In the initialize function, a plugin can be sure that the plugins it
      // depends on have initialized their members.

      new OrganizeIncludes::IncludesOrganizer (this);
      new CodeDiscover::CodeDiscover (this);
      new Ci::Ci (this);

      initTranslation ();
      return true;
    }

    void QtcUtilitiesPlugin::extensionsInitialized () {
      // Retrieve objects from the plugin manager's object pool
      // In the extensionsInitialized function, a plugin can be sure that all
      // plugins that depend on it are completely initialized.
    }

    ExtensionSystem::IPlugin::ShutdownFlag QtcUtilitiesPlugin::aboutToShutdown () {
      // Save settings
      // Disconnect from signals that are not needed during shutdown
      // Hide UI(if you add UI that is not in the main window directly)
      return SynchronousShutdown;
    }

    void QtcUtilitiesPlugin::initTranslation () {
      auto language = ICore::userInterfaceLanguage ();
      if (language.isEmpty ()) {
        return;
      }
      auto paths = QStringList () << ICore::resourcePath () << ICore::userResourcePath ();
      QString trFile = QLatin1String ("QtcUtilities_") + language;
      auto translator = new QTranslator (this);
      for (const auto &path: paths) {
        if (translator->load (trFile, path + QLatin1String ("/translations"))) {
          qApp->installTranslator (translator);
          break;
        }
      }
    }

  }
}

#include "ClassDiagramGenerator.h"
#include "CodeDiscover.h"
#include "CodeDiscoverConstants.h"
#include "CodeDiscoverOptionsPage.h"
#include "CodeDiscoverToolRunner.h"
#include "CodeDiscoverWindow.h"

#include <cplusplus/Symbol.h>

#include <extensionsystem/iplugin.h>

#include <coreplugin/imode.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>

#include <debugger/analyzer/analyzerutils.h>

#include <utils/icon.h>

#include <QMenu>


using namespace Core;
using namespace Utils;

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

CodeDiscover::CodeDiscover (ExtensionSystem::IPlugin *plugin) :
  options_ (new CodeDiscoverOptionsPage),
  window_ (new CodeDiscoverWindow (options_->classFlags ())),
  runner_ (new CodeDiscoverToolRunner (this)),
  classGenerator_ (new ClassDiagramGenerator (this))
{
  registerActions ();
  connect (options_.data (), &CodeDiscoverOptionsPage::settingsSaved,
           this, &CodeDiscover::updateSettings);
  plugin->addAutoReleasedObject (options_.data ());

  auto *mode = new IMode;
  mode->setId ({MODE_ID});
  mode->setContext (Context {CONTEXT_ID});
  mode->setWidget (window_.data ());
  auto iconName = Icon (QLatin1String (MODE_ICON));
  mode->setIcon (Icon::modeIcon (iconName, iconName, iconName));
  mode->setDisplayName (tr ("Discover"));
  plugin->addAutoReleasedObject (mode);

  connect (runner_, &CodeDiscoverToolRunner::newImage,
           this, &CodeDiscover::handleNewImage);
  connect (window_.data (), &CodeDiscoverWindow::flagsChanged,
           this, &CodeDiscover::updateClassFlags);
  updateSettings ();
}

void CodeDiscover::showEntryClassDiagram ()
{
  if (auto *symbol = AnalyzerUtils::findSymbolUnderCursor ()) {
    auto source = classGenerator_->generate (symbol, options_->classFlags ());
    if (!source.isEmpty ()) {
      runner_->convert (source);
    }
  }
}

void CodeDiscover::updateSettings ()
{
  const auto &settings = options_->settings ();
  runner_->setSettings (settings.javaBinary, settings.umlBinary, settings.dotBinary);
}

void CodeDiscover::handleNewImage (const QPixmap &image)
{
  if (window_ && !image.isNull ()) {
    window_->setImage (image);
    Core::ModeManager::activateMode ({MODE_ID});
  }
}

void CodeDiscover::updateClassFlags (ClassFlags flags)
{
  options_->setClassFlags (flags);
  showEntryClassDiagram ();
}

void CodeDiscover::registerActions ()
{
  auto menu = ActionManager::createMenu (MENU_ID);
  menu->menu ()->setTitle (tr ("Code discover"));
  ActionManager::actionContainer (Core::Constants::M_TOOLS)->addMenu (menu);

  {
    auto action = new QAction (tr ("Show class diagram"), this);
    connect (action, &QAction::triggered, this, &CodeDiscover::showEntryClassDiagram);
    auto command = ActionManager::registerAction (action, ACTION_SHOW_CLASS_DIAGRAM);
    command->setDefaultKeySequence (QKeySequence (tr ("Ctrl+Shift+A")));
    menu->addAction (command);
  }
}

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

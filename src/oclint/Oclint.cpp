#include "Oclint.h"
#include "OclintConstants.h"
#include "OclintRunner.h"
#include "OclintOptionsPage.h"

#include "../clangtools/AutoCheckEvents.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/taskhub.h>

#include <extensionsystem/iplugin.h>

#include <QAction>

using namespace ProjectExplorer;
using namespace Core;

namespace QtcUtilities {
  namespace Internal {
    namespace Oclint {

      Oclint::Oclint (ExtensionSystem::IPlugin *plugin)
        : checkEvents_ (new ClangTools::AutoCheckEvents (this)), options_ (new OclintOptionsPage) {
        registerActions ();

        ProjectExplorer::TaskHub::addCategory (TASK_CATEGORY_ID,
                                               QLatin1String (TASK_CATEGORY_NAME));

        plugin->addAutoReleasedObject (options_.data ());
        connect (options_.data (), &OclintOptionsPage::settingsSaved,
                 this, &Oclint::updateSettings);

        updateSettings ();
      }

      void Oclint::updateSettings () {
        QTC_ASSERT (options_, return );
        runner_ = RunnerPtr::create (nullptr);
        connect (checkEvents_, &ClangTools::AutoCheckEvents::check,
                 runner_.data (), &OclintRunner::check);
        runner_->setSettings (options_->settings ());
      }

      void Oclint::registerActions () {
        auto action = new QAction (tr ("Scan with oclint"), this);
        auto cmd = ActionManager::registerAction (action, ACTION_CHECK_NODE_ID);
        connect (action, &QAction::triggered,
                 checkEvents_, &ClangTools::AutoCheckEvents::checkCurrentNode);

        auto addToMenu = [cmd](const char *containerId, Command *command) {
                           if (auto menu = ActionManager::actionContainer (containerId)) {
                             menu->addAction (command);
                           }
                         };

        addToMenu (ProjectExplorer::Constants::M_FILECONTEXT, cmd);
        addToMenu (ProjectExplorer::Constants::M_FOLDERCONTEXT, cmd);
        addToMenu (ProjectExplorer::Constants::M_PROJECTCONTEXT, cmd);
        addToMenu (ProjectExplorer::Constants::M_SUBPROJECTCONTEXT, cmd);
      }

    } // namespace Oclint
  } // namespace Internal
} // namespace QtcUtilities

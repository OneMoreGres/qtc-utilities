#include "ClangTools.h"
#include "ClangToolsConstants.h"
#include "AutoCheckEvents.h"
#include "ToolRunner.h"
#include "ToolOptionsPage.h"

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
    namespace ClangTools {

      ClangTools::ClangTools (ExtensionSystem::IPlugin *plugin)
        : checkEvents_ (new AutoCheckEvents (this)), options_ (new ToolOptionsPage) {
        registerActions ();

        ProjectExplorer::TaskHub::addCategory (TASK_CATEGORY_ID,
                                               QLatin1String (TASK_CATEGORY_NAME));

        plugin->addAutoReleasedObject (options_.data ());
        connect (options_.data (), &ToolOptionsPage::settingsSaved,
                 this, &ClangTools::updateSettings);

        updateSettings ();
      }

      void ClangTools::updateSettings () {
        QTC_ASSERT (options_, return );
        runners_.clear ();
        const auto &settings = options_->settings ();
        for (const auto &setting: settings) {
          auto runner = RunnerPtr::create (nullptr);
          connect (checkEvents_, &AutoCheckEvents::check, runner.data (), &ToolRunner::check);
          runner->setSettings (setting);
          runners_ << runner;
        }
      }

      void ClangTools::registerActions () {
        auto action = new QAction (tr ("Scan with clang tools"), this);
        auto cmd = ActionManager::registerAction (action, ACTION_CHECK_NODE_ID);
        connect (action, &QAction::triggered, checkEvents_, &AutoCheckEvents::checkCurrentNode);

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

    } // namespace ClangTools
  } // namespace Internal
} // namespace QtcUtilities

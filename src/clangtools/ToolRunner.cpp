#include "ToolRunner.h"
#include "ClangToolsConstants.h"
#include "ToolOutputParser.h"

#include <coreplugin/messagemanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <coreplugin/progressmanager/futureprogress.h>
#include <utils/macroexpander.h>
#include <utils/qtcassert.h>

#include <QDebug>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QThread>

using namespace Core;

namespace QtcUtilities {
  namespace Internal {
    namespace ClangTools {

      ToolRunner::ToolRunner (QObject *parent) : QObject (parent),
        parser_ (new ToolOutputParser (this)), futureInterface_ (nullptr) {
#ifdef Q_OS_LINUX
        QProcess getConf;
        getConf.start (QLatin1String ("getconf ARG_MAX"));
        getConf.waitForFinished (2000);
        auto argMax = getConf.readAllStandardOutput ().replace ("\n", "");
        maxArgumentsLength_ = std::max (argMax.toInt (), 32000);
#else
        maxArgumentsLength_ = 32767;
#endif

        connect (&process_, SIGNAL (readyReadStandardOutput ()),
                 SLOT (readOutput ()));
        connect (&process_, SIGNAL (readyReadStandardError ()),
                 SLOT (readError ()));
        connect (&process_, SIGNAL (started ()),
                 SLOT (started ()));
        connect (&process_, SIGNAL (error (QProcess::ProcessError)),
                 SLOT (error (QProcess::ProcessError)));
        connect (&process_, SIGNAL (finished (int,QProcess::ExitStatus)),
                 SLOT (finished (int,QProcess::ExitStatus)));

        // Restart checking if got queue.
        connect (&process_, SIGNAL (finished (int,QProcess::ExitStatus)),
                 SLOT (checkQueued ()));
      }

      ToolRunner::~ToolRunner () {
        if (process_.isOpen ()) {
          process_.kill ();
        }
        queueTimer_.stop ();
        delete futureInterface_;
      }

      void ToolRunner::setSettings (const ToolRunnerSettings &settings) {
        settings_ = settings;
        if (settings_.name.isEmpty ()) {
          settings_.name = settings_.binary;
        }
        parser_->setToolName (settings_.name);
        fileCheckQueue_.clear ();
      }

      QStringList ToolRunner::filter (const QStringList &files) const {
        QStringList result;
        for (const auto &file: files) {
          auto extension = QFileInfo (file).completeSuffix ();
          if (settings_.extensions.contains (extension)) {
            result << file;
          }
        }
        return result;
      }

      void ToolRunner::check (const QStringList &files, int checkType) {
        if (!settings_.checkTypes.contains (checkType)) {
          return;
        }
        fileCheckQueue_ += filter (files);
        fileCheckQueue_.removeDuplicates ();
        if (process_.isOpen ()) {
          if (fileCheckQueue_ == currentlyCheckingFiles_) {
            process_.kill ();
            // Rechecking will be restarted on finish signal.
          }
          return;
        }
        // Delay helps to avoid double checking same file on editor change.
        const auto checkDelayInMs = 200;
        if (!queueTimer_.isActive ()) {
          queueTimer_.singleShot (checkDelayInMs, this, SLOT (checkQueued ()));
        }
      }

      void ToolRunner::stop () {
        fileCheckQueue_.clear ();
        if (process_.isOpen ()) {
          process_.kill ();
        }
      }

      void ToolRunner::checkQueued () {
        if (fileCheckQueue_.isEmpty () || settings_.binary.isEmpty ()) {
          return;
        }

        auto expander = Utils::globalMacroExpander ();
        auto expanded = expander->expand (settings_.arguments);
        QStringList arguments = expanded.split (QLatin1Char (' '), QString::SkipEmptyParts);

        // Limit args length
        currentlyCheckingFiles_.clear ();
        int argumentsLength = arguments.join (QLatin1String (" ")).length ();
        for (const auto &file: fileCheckQueue_) {
          argumentsLength += file.length () + 1; // +1 for separator
          if (argumentsLength < maxArgumentsLength_) {
            currentlyCheckingFiles_ << file;
          }
        }
        if (currentlyCheckingFiles_.size () == fileCheckQueue_.size ()) {
          fileCheckQueue_.clear ();
        }
        else {
          for (auto i = 0, end = currentlyCheckingFiles_.size (); i < end; ++i) {
            fileCheckQueue_.pop_front ();
          }
        }

        arguments += currentlyCheckingFiles_;
        parser_->setCheckingFiles (currentlyCheckingFiles_);

        auto binary = settings_.binary;
        process_.start (binary, arguments);
      }

      void ToolRunner::readOutput () {
        parser_->parseStandardOutput (process_.readAllStandardOutput ());
      }

      void ToolRunner::readError () {
        parser_->parseStandardError (process_.readAllStandardError ());
      }

      void ToolRunner::started () {
        delete futureInterface_;
        futureInterface_ = new QFutureInterface<void>;
        FutureProgress *progress = ProgressManager::addTask (futureInterface_->future (),
                                                             settings_.name, TASK_CHECKING);
        connect (progress, SIGNAL (canceled ()), SLOT (stop ()));
        futureInterface_->setProgressRange (0, 100); // %
        futureInterface_->reportStarted ();
      }

      void ToolRunner::error (QProcess::ProcessError error) {
        MessageManager::write (settings_.binary + tr (" error occured"), MessageManager::Silent);
        if (error == QProcess::FailedToStart) {
          finished (-1, QProcess::CrashExit);
        }
      }

      void ToolRunner::finished (int, QProcess::ExitStatus) {
        if (futureInterface_ != NULL) {
          futureInterface_->reportFinished ();
        }
        process_.close ();
      }

    } // namespace ClangTools
  } // namespace Internal
} // namespace QtcUtilities

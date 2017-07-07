#pragma once

#include <QProcess>
#include <QTimer>

#include <QFuture>

namespace QtcUtilities {
  namespace Internal {
    namespace ClangTools {

      class ToolOutputParser;

      struct ToolRunnerSettings {
        QString name;
        QString binary;
        QString arguments;
        QStringList extensions;
        QList<int> checkTypes;
      };

      class ToolRunner : public QObject {
        Q_OBJECT

        public:

          explicit ToolRunner (QObject *parent = 0);
          ~ToolRunner ();

          void setSettings (const ToolRunnerSettings &settings);

        public slots:
          //! Add files to check queue.
          void check (const QStringList &files, int checkType);
          //! Stop check progress if running and clear check queue.
          void stop ();

        private slots:
          //! Check files from queue.
          void checkQueued ();

          // QProcess handling.
          void readOutput ();
          void readError ();
          void started ();
          void error (QProcess::ProcessError error);
          void finished (int, QProcess::ExitStatus);

        private:
          QStringList filter (const QStringList &files) const;


          ToolOutputParser *parser_;
          //! Timer to delay queue checking.
          QTimer queueTimer_;
          //! Binary runner.
          QProcess process_;
          //! Plugin's settings.
          ToolRunnerSettings settings_;
          //! Queued list of files to check.
          QStringList fileCheckQueue_;
          //! List of files currently being checked.
          QStringList currentlyCheckingFiles_;
          //! Interface to inform about checking.
          QFutureInterface<void> *futureInterface_;
          //! Max summary arguments length.
          int maxArgumentsLength_;
      };

    } // namespace ClangTools
  } // namespace Internal
} // namespace QtcUtilities

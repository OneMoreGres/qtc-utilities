#pragma once

#include <QObject>
#include <QMap>
#include <QSet>

namespace QtcUtilities {
  namespace Internal {
    namespace Oclint {

      class ToolOutputParser : public QObject {
        Q_OBJECT

        public:
          explicit ToolOutputParser (QObject *parent = nullptr);

          void setCheckingFiles (const QStringList &files);
          void parseStandardError (const QByteArray &data);
          void parseStandardOutput (const QByteArray &data);

        private:
          void clearTasks (const QStringList &files);
          void parseOutput (const QByteArray &data);

          using TaskIds = QSet<unsigned>;
          using TaskIdsPerFile = QMap<QString, TaskIds>;
          TaskIdsPerFile taskIdsPerFile_;
      };

    } // namespace Oclint
  } // namespace Internal
} // namespace QtcUtilities

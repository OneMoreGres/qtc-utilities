#include "ToolOutputParser.h"
#include "ClangToolsConstants.h"

#include <projectexplorer/taskhub.h>

#include <QRegularExpression>

using namespace ProjectExplorer;

namespace QtcUtilities {
namespace Internal {
namespace ClangTools {

ToolOutputParser::ToolOutputParser (QObject *parent) : QObject (parent)
{
}

void ToolOutputParser::setCheckingFiles (const QStringList &files)
{
  clearTasks (files);
}

void ToolOutputParser::parseStandardError (const QByteArray & /*data*/)
{
}

void ToolOutputParser::parseStandardOutput (const QByteArray &data)
{
  enum {
    Full, File, Line, Column, Type, Message
  };
  QRegularExpression re (QLatin1String (R"((.+):(\d+):(\d+): (\w+):(.+))"));
  QMap<QString, Task::TaskType> types;
  types.insert (QLatin1String ("warning"), Task::Warning);
  types.insert (QLatin1String ("error"), Task::Error);

  auto lines = data.split ('\n');
  for (const auto &rawLine: lines) {
    QRegularExpressionMatch match = re.match (QString::fromUtf8 (rawLine));
    if (!match.hasMatch ()) {
      continue;
    }

    auto type = types.value (match.captured (Type), Task::Unknown);
    auto fileString = match.captured (File);
    auto file = Utils::FileName::fromString (fileString);
    auto line = match.captured (Line).toInt ();
    QString description = toolName_ + match.captured (Message);
    Task task (type, description, file, line, TASK_CATEGORY_ID);
    TaskHub::addTask (task);
    taskIdsPerFile_[fileString].insert (task.taskId);
  }
}

void ToolOutputParser::setToolName (const QString &toolName)
{
  toolName_ = QString (QLatin1String ("%1.%2: ")).arg (QLatin1String (TASK_CATEGORY_NAME))
              .arg (toolName);
}

void ToolOutputParser::clearTasks (const QStringList &files)
{
  for (const auto &file: files) {
    for (auto id: taskIdsPerFile_[file]) {
      Task task;
      task.taskId = id;
      TaskHub::removeTask (task);
    }
    taskIdsPerFile_.remove (file);
  }
}

} // namespace ClangTools
} // namespace Internal
} // namespace QtcUtilities

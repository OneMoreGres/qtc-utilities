#include "CodeDiscoverToolRunner.h"

#include <QPixmap>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

CodeDiscoverToolRunner::CodeDiscoverToolRunner (QObject *parent) : QObject (parent)
{
  connect (&process_, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
           this, &CodeDiscoverToolRunner::handleFinish);
}

void CodeDiscoverToolRunner::setSettings (const QString &java, const QString &tool,
                                          const QString &dot)
{
  command_ = java;
  arguments_.clear ();
  arguments_ << QStringLiteral ("-jar") << tool << QStringLiteral ("-pipe")
             << QStringLiteral ("-graphvizdot") << dot << QStringLiteral ("-tpng")
             << QStringLiteral ("-nbthread") << QStringLiteral ("auto");
}

void CodeDiscoverToolRunner::convert (const QString &text)
{
  if (process_.isOpen ()) {
    process_.terminate ();
  }
  process_.start (command_, arguments_);
  if (!process_.waitForStarted (2000)) {
    return;
  }
  process_.write (text.toUtf8 ());
  process_.closeWriteChannel ();
}

void CodeDiscoverToolRunner::handleFinish (int /*exitCode*/,
                                           QProcess::ExitStatus /*exitStatus*/)
{
  auto data = process_.readAllStandardOutput ();
  QPixmap picture;
  if (picture.loadFromData (data, "PNG")) {
    emit newImage (picture);
  }
}

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

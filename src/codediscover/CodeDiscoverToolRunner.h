#pragma once

#include <QObject>
#include <QPixmap>
#include <QProcess>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

class CodeDiscoverToolRunner : public QObject
{
  Q_OBJECT

  public:
    explicit CodeDiscoverToolRunner (QObject *parent = 0);
    void setSettings (const QString &java, const QString &tool, const QString &dot);

  public slots:
    void convert (const QString &text);

  signals:
    void newImage (const QPixmap &image);

  private slots:
    void handleFinish (int exitCode, QProcess::ExitStatus exitStatus);

  private:
    QString command_;
    QStringList arguments_;
    QProcess process_;
};

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

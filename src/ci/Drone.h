#pragma once

#include "ModelItem.h"

#include <QUrl>
#include <QNetworkAccessManager>

class QJsonObject;

namespace QtcUtilities {
namespace Internal {
namespace Ci {
namespace Drone {

class Node : public QObject, public ModelItem
{
  Q_OBJECT

  public:
    explicit Node (ModelItem &parent);

  signals:
    void updated (ModelItem *item);
    void added (ModelItem *item);

  protected:
    void timerEvent (QTimerEvent *e) override;

  private slots:
    void replyFinished (QNetworkReply *reply);

  private:
    void login ();
    void getReposotories ();
    void parseRepositories (const QByteArray &reply);
    void getBuilds (ModelItem &repository);
    void updateBuild (ModelItem &build);
    void parseBuilds (const QByteArray &reply, ModelItem &repository);
    void parseBuild (const QJsonObject &object, ModelItem &build);

    QUrl url_ {};
    QByteArray user_ {};
    QByteArray pass_ {};

    QList<QNetworkReply *> pendingReplies_;
    QNetworkAccessManager *manager_;
};

} // namespace Drone
} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

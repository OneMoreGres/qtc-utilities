#include "Drone.h"

#include <coreplugin/messagemanager.h>

#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace {

enum class RequestType {
  Login, GetRepositories, GetBuilds, GetJobs, GetLogs
};
enum class Attribute {
  RequestType = QNetworkRequest::User,
  Context
};

enum NodeField {
  NodeFieldUrl, NodeFieldUser
};
enum RepoField {
  RepoFieldName, RepoFieldStatus, RepoFieldLastStarted, RepoFieldLastFinished,
  RepoFieldLastBranch, RepoFieldLastAuthor, RepoFieldLastMessage
};
enum BuildField {
  BuildFieldNumber, BuildFieldStatus, BuildFieldStarted, BuildFieldFinished,
  BuildFieldBranch, BuildFieldAuthor, BuildFieldMessage
};
enum JobField {
  JobFieldNumber, JobFieldStatus, JobFieldStarted, JobFieldFinished
};


}
Q_DECLARE_METATYPE (RequestType)

namespace QtcUtilities {
namespace Internal {
namespace Ci {
namespace Drone {

Node::Node (ModelItem &parent)
  : ModelItem (&parent),
  manager_ (new QNetworkAccessManager (this))
{
  setData (NodeFieldUrl, url_);
  setData (NodeFieldUser, user_);
  connect (manager_, &QNetworkAccessManager::finished,
           this, &Node::replyFinished);
  login ();

  startTimer (3000);
}

void Node::timerEvent (QTimerEvent */*e*/)
{
  if (!pendingReplies_.isEmpty ()) {
    return;
  }

  for (auto repo: children_) {
    getBuilds (*repo);
  }

}

void Node::replyFinished (QNetworkReply *reply)
{
  pendingReplies_.removeAll (reply);

  auto request = reply->request ();
  auto requestType = request.attribute (QNetworkRequest::Attribute (Attribute::RequestType))
                     .value<RequestType>();

  if (reply->error () != QNetworkReply::NoError) {
    qCritical () << "reply error" << reply->errorString () << int (reply->error ())
                 << "on url" << request.url ();
    reply->deleteLater ();
    return;
  }

  switch (requestType) {
    case RequestType::Login:
      getReposotories ();
      break;

    case RequestType::GetRepositories:
      parseRepositories (reply->readAll ());
      break;

    case RequestType::GetBuilds:
      {
        auto context = request.attribute (QNetworkRequest::Attribute (Attribute::Context));
        auto *repository = context.value <ModelItem *> ();
        if (!repository) {
          qCritical () << "repository item is empty";
          break;
        }
        parseBuilds (reply->readAll (), *repository);
      }
      break;

    case RequestType::GetJobs:
      {
        auto context = request.attribute (QNetworkRequest::Attribute (Attribute::Context));
        auto *build = context.value <ModelItem *> ();
        if (!build) {
          qCritical () << "build item is empty";
          break;
        }
        parseJobs (reply->readAll (), *build);
      }
      break;

    case RequestType::GetLogs:
      {
        auto log = reply->readAll ();
        if (!log.isEmpty ()) {
          QString msg = ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n" + QString::fromUtf8 (log)
                        + ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
          Core::MessageManager::write (msg, Core::MessageManager::WithFocus);
        }
      }
      break;

    default:
      Q_ASSERT_X (false, "request", "unhandled request type");
  }

  reply->deleteLater ();
}

void Node::login ()
{
  auto *multiPart = new QHttpMultiPart (QHttpMultiPart::FormDataType);
  QHttpPart user;
  user.setHeader (QNetworkRequest::ContentDispositionHeader,
                  QVariant ("form-data; name=\"username\""));
  user.setBody (user_);
  multiPart->append (user);

  QHttpPart pass;
  pass.setHeader (QNetworkRequest::ContentDispositionHeader,
                  QVariant ("form-data; name=\"password\""));
  pass.setBody (pass_);
  multiPart->append (pass);

  auto url = url_;
  url.setPath ("/authorize");
  QNetworkRequest request (url);
  request.setAttribute (QNetworkRequest::Attribute (Attribute::RequestType),
                        QVariant::fromValue (RequestType::Login));

  QNetworkReply *reply = manager_->post (request, multiPart);
  multiPart->setParent (reply);
  pendingReplies_ << reply;
}

void Node::getReposotories ()
{
  auto url = url_;
  url.setPath ("/api/user/repos");
  QNetworkRequest request (url);
  request.setAttribute (QNetworkRequest::Attribute (Attribute::RequestType),
                        QVariant::fromValue (RequestType::GetRepositories));

  pendingReplies_ << manager_->get (request);
}

void Node::parseRepositories (const QByteArray &reply)
{
  auto doc = QJsonDocument::fromJson (reply);
  if (!doc.isArray ()) {
    qCritical () << "wrong repositories list format" << reply;
    return;
  }
  for (QJsonValueRef value: doc.array ()) {
    auto repo = QSharedPointer<ModelItem>::create (this);

    auto object = value.toObject ();
    auto name = object["full_name"].toString ();
    repo->setData (RepoFieldName, name);

    children_ << repo;
    emit added (repo.data ());

    getBuilds (*repo);
  }
}

void Node::getBuilds (ModelItem &repository)
{
  auto url = repository.parent ()->data (NodeFieldUrl).toUrl ();
  url.setPath ("/api/repos/" + repository.data (RepoFieldName).toString () + "/builds");
  QNetworkRequest request (url);
  request.setAttribute (QNetworkRequest::Attribute (Attribute::RequestType),
                        QVariant::fromValue (RequestType::GetBuilds));
  request.setAttribute (QNetworkRequest::Attribute (Attribute::Context),
                        QVariant::fromValue (&repository));

  pendingReplies_ << manager_->get (request);
}

void Node::parseBuilds (const QByteArray &reply, ModelItem &repository)
{
  auto doc = QJsonDocument::fromJson (reply);
  if (!doc.isArray ()) {
    qCritical () << "wrong builds list format" << reply;
    return;
  }
  for (QJsonValueRef value: doc.array ()) {
    auto object = value.toObject ();
    auto number = object["number"].toInt ();

    auto found = false;
    for (auto build: repository.children ()) {
      if (number == build->data (BuildFieldNumber).toInt ()) {
        if (build->decoration () == Decoration::Working) {
          parseBuild (object, *build);
          updateRepository (repository, *build);
        }
        found = true;
        break;
      }
    }

    if (found) {
      continue;
    }

    auto build = QSharedPointer<ModelItem>::create (&repository);
    parseBuild (object, *build);
    repository.addChild (build);
    emit added (build.data ());
  }
}

void Node::parseBuild (const QJsonObject &object, ModelItem &build)
{
  build.setData (BuildFieldNumber, object["number"].toInt ());
  build.setData (BuildFieldStatus, object["status"].toString ());
  build.setData (BuildFieldStarted,
                 QDateTime::fromTime_t (object["started_at"].toVariant ().toUInt ()));
  build.setData (BuildFieldFinished,
                 QDateTime::fromTime_t (object["finished_at"].toVariant ().toUInt ()));
  build.setData (BuildFieldBranch, object["branch"].toString ());
  build.setData (BuildFieldAuthor, object["author"].toString ());
  build.setData (BuildFieldMessage, object["message"].toString ().trimmed ());

  QMap<QString, ModelItem::Decoration> decorations {
    {"success", ModelItem::Decoration::Success},
    {"failure", ModelItem::Decoration::Failure},
    {"working", ModelItem::Decoration::Working}
  };
  auto status = build.data (BuildFieldStatus).toString ();
  auto decoration = decorations.value (status);
  build.setDecoration (decoration);
  if (decoration == ModelItem::Decoration::Failure) {
    getJobs (build);
  }

  updateRepository (*build.parent (), build);
}

void Node::updateRepository (ModelItem &repository, const ModelItem &build)
{
  auto lastStarted = repository.data (RepoFieldLastStarted).toDateTime ();
  auto started = build.data (BuildFieldStarted).toDateTime ();
  if (lastStarted < started) {
    repository.setData (RepoFieldLastBranch, build.data (BuildFieldBranch));
    repository.setData (RepoFieldLastAuthor, build.data (BuildFieldAuthor));
    repository.setData (RepoFieldLastMessage, build.data (BuildFieldMessage));
    repository.setData (RepoFieldLastStarted, started);
    repository.setData (RepoFieldLastFinished, build.data (BuildFieldFinished));
    repository.setDecoration (build.decoration ());
    emit updated (&repository);
  }
}

void Node::getJobs (ModelItem &build)
{
  auto repository = *build.parent ();
  auto url = repository.parent ()->data (NodeFieldUrl).toUrl ();
  url.setPath ("/api/repos/" + repository.data (RepoFieldName).toString () + "/builds/"
               + build.data (BuildFieldNumber).toString ());
  QNetworkRequest request (url);
  request.setAttribute (QNetworkRequest::Attribute (Attribute::RequestType),
                        QVariant::fromValue (RequestType::GetJobs));
  request.setAttribute (QNetworkRequest::Attribute (Attribute::Context),
                        QVariant::fromValue (&build));

  pendingReplies_ << manager_->get (request);
}

void Node::parseJobs (const QByteArray &reply, ModelItem &build)
{
  auto doc = QJsonDocument::fromJson (reply);
  if (!doc.isObject ()) {
    qCritical () << "wrong build info format" << reply;
    return;
  }

  auto object = doc.object ();

  for (QJsonValueRef value: object["jobs"].toArray ()) {
    auto object = value.toObject ();
    auto number = object["number"].toInt ();

    auto found = false;
    for (auto job: build.children ()) {
      if (number == job->data (JobFieldNumber).toInt ()) {
        parseJob (object, *job);
        found = true;
        break;
      }
    }

    if (found) {
      continue;
    }

    auto job = QSharedPointer<ModelItem>::create (&build);
    parseJob (object, *job);
    build.addChild (job);
    emit added (job.data ());
  }
}

void Node::parseJob (const QJsonObject &object, ModelItem &job)
{
  job.setData (JobFieldNumber, object["number"].toInt ());
  job.setData (JobFieldStatus, object["status"].toString ());
  job.setData (JobFieldStarted,
               QDateTime::fromTime_t (object["started_at"].toVariant ().toUInt ()));
  job.setData (JobFieldFinished,
               QDateTime::fromTime_t (object["finished_at"].toVariant ().toUInt ()));

  QMap<QString, ModelItem::Decoration> decorations {
    {"success", ModelItem::Decoration::Success},
    {"failure", ModelItem::Decoration::Failure},
    {"working", ModelItem::Decoration::Working}
  };
  auto status = job.data (JobFieldStatus).toString ();
  auto decoration = decorations.value (status);
  job.setDecoration (decoration);
}

void Node::getLogs (ModelItem &job)
{
  const auto build = *job.parent ();
  const auto repository = *build.parent ();
  auto url = repository.parent ()->data (NodeFieldUrl).toUrl ();
  url.setPath ("/api/repos/" + repository.data (RepoFieldName).toString () + "/logs/"
               + build.data (BuildFieldNumber).toString () + "/"
               + job.data (JobFieldNumber).toString ());
  QNetworkRequest request (url);
  request.setAttribute (QNetworkRequest::Attribute (Attribute::RequestType),
                        QVariant::fromValue (RequestType::GetLogs));

  pendingReplies_ << manager_->get (request);
}

} // namespace Drone
} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

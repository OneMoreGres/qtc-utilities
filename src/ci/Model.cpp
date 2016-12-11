#include "Model.h"
#include "Drone.h"
#include "NodeEdit.h"

#include <projectexplorer/session.h>

#include <QAbstractItemView>
#include <QMenu>


namespace QtcUtilities {
namespace Internal {
namespace Ci {

Model::Model (QObject *parent)
  : QAbstractItemModel (parent),
  root_ (new ModelItem (nullptr))
{
  using ProjectExplorer::SessionManager;
  auto *session = SessionManager::instance ();
  connect (session, &SessionManager::aboutToSaveSession, this, &Model::saveSession);
  connect (session, &SessionManager::aboutToLoadSession, this, &Model::loadSession);

  header_ = QStringList {tr ("Name"), tr ("Status"), tr ("Started"), tr ("Finished"),
                         tr ("Branch"), tr ("Author"), tr ("Message")};
}

Model::~Model ()
{
}

ModelItem * Model::item (const QModelIndex &index) const
{
  auto *ptr = (index.isValid ()
               ? reinterpret_cast<ModelItem *>(index.internalPointer ())
               : root_.data ());
  Q_ASSERT (ptr);
  return ptr;
}

QModelIndex Model::index (ModelItem *item, int column) const
{
  if (!item) {
    return {};
  }
  auto row = item->row ();
  if (row != -1) {
    return createIndex (row, column, item);
  }
  return {};
}

QModelIndex Model::index (int row, int column, const QModelIndex &parent) const
{
  auto *ptr = item (parent);
  auto result = index (ptr->child (row), column);
  return result;
}

QModelIndex Model::parent (const QModelIndex &child) const
{
  auto *ptr = item (child);
  auto result = index (ptr->parent ());
  return result;
}

int Model::rowCount (const QModelIndex &parent) const
{
  auto *ptr = item (parent);
  return ptr->rowCount ();
}

int Model::columnCount (const QModelIndex &parent) const
{
  if (!parent.isValid ()) {
    return header_.size ();
  }
  auto *ptr = item (parent);
  return ptr->columnCount ();
}

QVariant Model::data (const QModelIndex &index, int role) const
{
  auto *ptr = item (index);
  return ptr->data (index.column (), role);
}

QVariant Model::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole) {
    return {};
  }
  if (orientation == Qt::Vertical) {
    return section + 1;
  }
  if (section < header_.size ()) {
    return header_[section];
  }
  return section + 1;
}

void Model::contextMenu (const QPoint &point)
{
  auto *view = qobject_cast<QAbstractItemView *> (sender ());
  if (!view) {
    return;
  }
  auto index = view->indexAt (point);
  if (!index.isValid ()) {
    QMenu menu;
    auto *addNodeAction = menu.addAction (tr ("Add node"));

    auto *choice = menu.exec (QCursor::pos ());

    if (choice == addNodeAction) {
      NodeEdit edit;
      auto res = edit.exec ();
      if (res == QDialog::Accepted) {
        if (edit.mode () == NodeEdit::Mode::Drone) {
          addNode (edit.drone ());
        }
      }
    }
    return;
  }
  emit requestContextMenu (item (index));
}

void Model::addNode (const Drone::Settings &settings)
{
  auto row = root_->rowCount ();
  beginInsertRows ({}, row, row);
  auto node = QSharedPointer<Drone::Node>::create (*root_, settings);
  connect (node.data (), &Drone::Node::added, this, &Model::add);
  connect (node.data (), &Drone::Node::prepended, this, &Model::prepend);
  connect (node.data (), &Drone::Node::updated, this, &Model::update);
  connect (node.data (), &Drone::Node::reset, this, &Model::reset);
  connect (this, &Model::requestContextMenu, node.data (), &Drone::Node::contextMenu);
  root_->addChild (node);
  endInsertRows ();
}

void Model::saveSession ()
{
  QVariantList settings;
  for (auto child: root_->children ()) {
    if (auto drone = child.dynamicCast<Drone::Node>()) {
      settings << drone->settings ().toVariant ();
    }
  }
  ProjectExplorer::SessionManager::setValue ("ci_settings", settings);
}

void Model::loadSession ()
{
  beginResetModel ();
  root_->clear ();
  auto settings = ProjectExplorer::SessionManager::value ("ci_settings").toList ();
  for (const auto &i: settings) {
    {
      auto drone = Drone::Settings::fromVariant (i);
      if (drone.isValid ()) {
        addNode (drone);
      }
    }
  }
  endResetModel ();
}

void Model::prepend (ModelItem *item)
{
  auto parentIndex = parent (index (item));
  beginInsertRows (parentIndex, 0, 0);
  endInsertRows ();
}

void Model::add (ModelItem *item)
{
  auto parentIndex = parent (index (item));
  auto row = item->row ();
  beginInsertRows (parentIndex, row, row);
  endInsertRows ();
}

void Model::update (ModelItem *item)
{
  auto left = index (item);
  emit dataChanged (left, left.sibling (left.row (), 1));
}

void Model::reset ()
{
  beginResetModel ();
  endResetModel ();
}

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

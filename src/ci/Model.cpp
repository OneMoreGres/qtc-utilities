#include "Model.h"
#include "Drone.h"

#include <QAbstractItemView>


namespace QtcUtilities {
namespace Internal {
namespace Ci {

Model::Model (QObject *parent)
  : QAbstractItemModel (parent),
  root_ (new ModelItem (nullptr))
{
  auto node = QSharedPointer<Drone::Node>::create (*root_);
  connect (node.data (), &Drone::Node::added, this, &Model::add);
  connect (node.data (), &Drone::Node::updated, this, &Model::update);
  connect (this, &Model::requestContextMenu, node.data (), &Drone::Node::contextMenu);
  root_->addChild (node);


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
    return;
  }
  emit requestContextMenu (item (index));
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

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

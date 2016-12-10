#pragma once

#include <QAbstractItemModel>

namespace QtcUtilities {
namespace Internal {
namespace Ci {

class ModelItem;

class Model : public QAbstractItemModel
{
  Q_OBJECT

  public:

    explicit Model (QObject *parent = 0);
    ~Model () override;

    QModelIndex index (int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent (const QModelIndex &child) const override;
    int rowCount (const QModelIndex &parent) const override;
    int columnCount (const QModelIndex &parent) const override;
    QVariant data (const QModelIndex &index, int role) const override;
    QVariant headerData (int section, Qt::Orientation orientation, int role) const override;

  signals:
    void requestContextMenu (ModelItem *item);

  public slots:
    void contextMenu (const QPoint &point);

  private slots:
    void add (ModelItem *item);
    void update (ModelItem *item);
    void reset ();

  private:
    QModelIndex index (ModelItem *item, int column = 0) const;
    ModelItem * item (const QModelIndex &index) const;

    QScopedPointer<ModelItem> root_;
    QStringList header_;
};

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

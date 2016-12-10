#pragma once

#include <QMetaType>
#include <QVariant>

namespace QtcUtilities {
namespace Internal {
namespace Ci {

class ModelItem
{
  public:
    enum class Decoration {
      None, Success, Failure, Working
    };
    using Data = QVariantList;
    ModelItem (ModelItem *parent);
    virtual ~ModelItem ();

    ModelItem * parent () const;
    ModelItem * child (int row) const;
    void addChild (QSharedPointer<ModelItem> child);
    int row () const;
    int rowCount () const;
    int columnCount () const;

    void setDecoration (Decoration decoration);
    void setData (int column, const QVariant &data);
    QVariant data (int column, int role = Qt::DisplayRole) const;

  protected:
    ModelItem *parent_;
    Data data_;
    Decoration decoration_;
    QList<QSharedPointer<ModelItem> > children_;
};

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

Q_DECLARE_METATYPE (QtcUtilities::Internal::Ci::ModelItem *)

#pragma once

#include "CodeDiscoverSettings.h"

#include <QLabel>
#include <QMap>

QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

class CodeDiscoverWindow : public QWidget
{
  Q_OBJECT

  public:
    CodeDiscoverWindow (ClassFlags flags, QWidget *parent = nullptr,
                        Qt::WindowFlags f = {});

  signals:
    void flagsChanged (ClassFlags flags);

  public slots:
    void setImage (const QPixmap &image);

  private slots:
    void updateFlags ();
    void saveToFile ();

  private:
    QLabel *imageLabel_;
    QMap < ClassFlags, QCheckBox *> flags_;
};

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

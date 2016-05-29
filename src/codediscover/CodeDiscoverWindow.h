#pragma once

#include <QLabel>
#include <QWidget>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

class CodeDiscoverWindow : public QWidget
{
  Q_OBJECT

  public:
    CodeDiscoverWindow (QWidget *parent = nullptr, Qt::WindowFlags f = {});

  public slots:
    void setImage (const QPixmap &image);

  private:
    QLabel *imageLabel_;
};

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

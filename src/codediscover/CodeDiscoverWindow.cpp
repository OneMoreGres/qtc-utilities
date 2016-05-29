#include "CodeDiscoverWindow.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QDebug>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

CodeDiscoverWindow::CodeDiscoverWindow (QWidget *parent, Qt::WindowFlags f)
  : QWidget (parent, f),
  imageLabel_ (nullptr)
{
  auto layout = new QGridLayout;

  imageLabel_ = new QLabel;
  imageLabel_->setBackgroundRole (QPalette::Base);
  imageLabel_->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
  imageLabel_->setScaledContents (true);

  auto row = 0;
  auto *scrollArea = new QScrollArea;
  scrollArea->setBackgroundRole (QPalette::Dark);
  scrollArea->setWidget (imageLabel_);
  layout->addWidget (scrollArea, row, 0);

  setLayout (layout);
}

void CodeDiscoverWindow::setImage (const QPixmap &image)
{
  imageLabel_->setPixmap (image);
  imageLabel_->adjustSize ();
}

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

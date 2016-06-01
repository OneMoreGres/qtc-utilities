#include "CodeDiscoverWindow.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDebug>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

CodeDiscoverWindow::CodeDiscoverWindow (ClassFlags flags, QWidget *parent,
                                        Qt::WindowFlags f)
  : QWidget (parent, f),
  imageLabel_ (nullptr)
{
  auto flagsLayout = new QVBoxLayout;
  flagsLayout->addWidget (new QLabel (tr ("Settings")));
#define ADD_CHECKBOX(FLAG, TITLE) flagsLayout->addWidget (flags_.insert (FLAG, new QCheckBox (tr (TITLE))).value ());
  ADD_CHECKBOX (ShowMembers, "Show members");
  ADD_CHECKBOX (ShowMethods, "Show methods");
  ADD_CHECKBOX (ShowPublic, "Show public");
  ADD_CHECKBOX (ShowProtected, "Show protected");
  ADD_CHECKBOX (ShowPrivate, "Show private");
  ADD_CHECKBOX (ShowBase, "Show base classes");
  ADD_CHECKBOX (ShowDerived, "Show derived classes");
  ADD_CHECKBOX (ShowDependencies, "Show dependencies");
  ADD_CHECKBOX (ShowOnlyHierarchyDependencies, "Show only base and derived dependencies");
  ADD_CHECKBOX (ShowDependsDetails, "Show dependencies details");
  ADD_CHECKBOX (ShowHierarchyDetails, "Show base and derived details");
  ADD_CHECKBOX (ShowNested, "Show nested classes");
#undef ADD_CHECKBOX
  flagsLayout->addStretch (1);
  auto saveButton = new QPushButton (tr ("Save to file..."));
  connect (saveButton, &QPushButton::clicked, this, &CodeDiscoverWindow::saveToFile);
  flagsLayout->addWidget (saveButton);

  auto layout = new QGridLayout;

  imageLabel_ = new QLabel;
  imageLabel_->setBackgroundRole (QPalette::Base);
  imageLabel_->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
  imageLabel_->setScaledContents (true);

  auto row = 0;
  auto *scrollArea = new QScrollArea;
  scrollArea->setBackgroundRole (QPalette::Dark);
  scrollArea->setWidget (imageLabel_);
  layout->addLayout (flagsLayout, row, 0);
  layout->addWidget (scrollArea, row, 1);

  setLayout (layout);

  for (auto flag: flags_.keys ()) {
    auto *check = flags_[flag];
    if (flags & flag) {
      check->setChecked (true);
    }
    connect (check, &QCheckBox::clicked, this, &CodeDiscoverWindow::updateFlags);
  }
}

void CodeDiscoverWindow::setImage (const QPixmap &image)
{
  imageLabel_->setPixmap (image);
  imageLabel_->adjustSize ();
}

void CodeDiscoverWindow::updateFlags ()
{
  auto flags = ShowNothing;
  for (auto flag: flags_.keys ()) {
    if (flags_[flag]->isChecked ()) {
      flags = ClassFlags (flags | flag);
    }
  }
  emit flagsChanged (flags);
}

void CodeDiscoverWindow::saveToFile ()
{
  QString extension = QStringLiteral (".png");
  auto file = QFileDialog::getSaveFileName (this, {}, {}, QStringLiteral ("*.") + extension);
  if (!file.isEmpty ()) {
    if (!file.endsWith (extension)) {
      file += extension;
    }
    imageLabel_->pixmap ()->save (file, "PNG");
  }
}

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

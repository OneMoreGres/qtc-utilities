#include "NodeEdit.h"
#include "Drone.h"

#include <QGridLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace QtcUtilities {
namespace Internal {
namespace Ci {

NodeEdit::NodeEdit (QWidget *parent)
  : QDialog (parent),
  mode_ (Mode::Drone), url_ (new QLineEdit (this)),
  user_ (new QLineEdit (this)), pass_ (new QLineEdit (this)),
  savePass_ (new QCheckBox (tr ("Save password"), this))
{
  auto *layout = new QGridLayout (this);
  auto row = 0;
  layout->addWidget (new QLabel (tr ("Url")), row, 0);
  layout->addWidget (url_, row, 1);

  ++row;
  layout->addWidget (new QLabel (tr ("Username")), row, 0);
  layout->addWidget (user_, row, 1);

  ++row;
  layout->addWidget (new QLabel (tr ("Password")), row, 0);
  layout->addWidget (pass_, row, 1);

  ++row;
  layout->addWidget (savePass_, row, 0, 1, -1);

  ++row;
  auto *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  layout->addWidget (buttons, row, 0, 1, -1);
  connect (buttons, &QDialogButtonBox::accepted, this, &NodeEdit::accept);
  connect (buttons, &QDialogButtonBox::rejected, this, &NodeEdit::reject);
}

void NodeEdit::setDrone (Drone::Settings &drone)
{
  setMode (Mode::Drone);
  url_->setText (drone.url.toString ());
  user_->setText (QString::fromUtf8 (drone.user));
  pass_->setText (QString::fromUtf8 (drone.pass));
  savePass_->setChecked (drone.savePass);
}

Drone::Settings NodeEdit::drone () const
{
  if (mode_ == Mode::Drone) {
    return {QUrl (url_->text ()), user_->text ().toUtf8 (), pass_->text ().toUtf8 (),
            savePass_->isChecked ()};
  }
  return {};
}

NodeEdit::Mode NodeEdit::mode () const
{
  return mode_;
}

void NodeEdit::setMode (Mode mode)
{
  mode_ = mode;
}

bool NodeEdit::savePass () const
{
  return savePass_->isChecked ();
}


} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

#include "Pane.h"
#include "Model.h"

namespace QtcUtilities {
namespace Internal {
namespace Ci {

Pane::Pane ()
  : widget_ (new QTreeView), model_ (new Model (this))
{
  widget_->setModel (model_);
  widget_->setEditTriggers (QAbstractItemView::NoEditTriggers);
}

Pane::~Pane ()
{
  delete widget_;
}

QWidget * Pane::outputWidget (QWidget */*parent*/)
{
  return widget_;
}

QList<QWidget *> Pane::toolBarWidgets () const
{
  return {};
}

QString Pane::displayName () const
{
  return tr ("Continuous integration");
}

int Pane::priorityInStatusBar () const
{
  return 90;
}

void Pane::clearContents ()
{
}

void Pane::visibilityChanged (bool visible)
{
  widget_->setVisible (visible);
}

void Pane::setFocus ()
{
  widget_->setFocus ();
}

bool Pane::hasFocus () const
{
  return widget_->hasFocus ();
}

bool Pane::canFocus () const
{
  return true;
}

bool Pane::canNavigate () const
{
  return false;
}

bool Pane::canNext () const
{
  return false;
}

bool Pane::canPrevious () const
{
  return false;
}

void Pane::goToNext ()
{
}

void Pane::goToPrev ()
{
}

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

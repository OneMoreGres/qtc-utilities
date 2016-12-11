#pragma once

#include <coreplugin/ioutputpane.h>

#include <QTreeView>

namespace QtcUtilities {
namespace Internal {
namespace Ci {

class Model;

class Pane : public Core::IOutputPane
{
  Q_OBJECT

  public:
    Pane ();
    ~Pane ();

    QWidget * outputWidget (QWidget *parent) override;
    QList<QWidget *> toolBarWidgets () const override;
    QString displayName () const override;
    int priorityInStatusBar () const override;
    void clearContents () override;
    void visibilityChanged (bool visible) override;
    void setFocus () override;
    bool hasFocus () const override;
    bool canFocus () const override;
    bool canNavigate () const override;
    bool canNext () const override;
    bool canPrevious () const override;
    void goToNext () override;
    void goToPrev () override;

  private:
    QTreeView *widget_;
    Model *model_;
};

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

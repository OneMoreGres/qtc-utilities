#pragma once

#include <coreplugin/ioutputpane.h>

class QMainWindow;
class QToolButton;

namespace ProjectExplorer {
class RunControl;
}

namespace QtcUtilities {
namespace Internal {
namespace DockedOutput {

class DockedOutputPane : public Core::IOutputPane
{
  Q_OBJECT

  public:
    DockedOutputPane ();
    ~DockedOutputPane () override;

  public:
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

  public slots:
    void handleRunControlStart (ProjectExplorer::RunControl *rc);

  private slots:
    void setIsEnabled (bool isEnabled);

  private:
    QMainWindow *widget_;
    QToolButton *isEnabledButton_;

    bool isEnabled_;
};

} // namespace DockedOutput
} // namespace Internal
} // namespace QtcUtilities

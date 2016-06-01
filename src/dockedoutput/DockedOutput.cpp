#include "DockedOutput.h"


#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorersettings.h>
#include <projectexplorer/runconfiguration.h>

#include <coreplugin/outputwindow.h>
#include <coreplugin/find/basetextfind.h>
#include <coreplugin/coreicons.h>

#include <texteditor/fontsettings.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/behaviorsettings.h>

#include <extensionsystem/pluginmanager.h>

#include <utils/outputformatter.h>


#include <QDockWidget>
#include <QMainWindow>
#include <QToolButton>

using namespace Core;
using namespace ProjectExplorer;
using namespace TextEditor;

namespace QtcUtilities {
namespace Internal {
namespace DockedOutput {
const auto CONTEXT_ID = "DockedOutput.Context";


class DockedOutputWidget : public QDockWidget
{
  Q_OBJECT

  public:
    DockedOutputWidget (int id, RunControl *rc, QWidget *parent = nullptr);
    ~DockedOutputWidget () override;

    bool reuse (RunControl *rc);

  public slots:
    void appendMessage (RunControl *rc, const QString &out, Utils::OutputFormat format);
    void markReady ();

  private:
    OutputWindow *output_;
    QPointer<RunConfiguration> configuration_;
    bool isReady_;
};


DockedOutputWidget::DockedOutputWidget (int id, RunControl *rc, QWidget *parent)
  : QDockWidget (parent),
  output_ (new OutputWindow (Context (Id (CONTEXT_ID).withSuffix (id)), this)),
  configuration_ (rc->runConfiguration ()), isReady_ (true)
{
  setWindowTitle (rc->displayName ());
  setAllowedAreas (Qt::AllDockWidgetAreas);
  setWidget (output_);

  auto formatter = new Utils::OutputFormatter;
  formatter->setParent (this);
  output_->setFormatter (formatter);
  output_->setWordWrapEnabled (ProjectExplorerPlugin::projectExplorerSettings ().wrapAppOutput);
  output_->setMaxLineCount (ProjectExplorerPlugin::projectExplorerSettings ().maxAppOutputLines);
  output_->setWheelZoomEnabled (TextEditorSettings::behaviorSettings ().m_scrollWheelZooming);
  output_->setBaseFont (TextEditorSettings::fontSettings ().font ());

  auto agg = new Aggregation::Aggregate (this);
  agg->add (output_);
  agg->add (new BaseTextFind (output_));

  reuse (rc);
}

DockedOutputWidget::~DockedOutputWidget ()
{

}

bool DockedOutputWidget::reuse (RunControl *rc)
{
  if (!isReady_ || rc->runConfiguration () != configuration_.data ()) {
    return false;
  }

  output_->grayOutOldContent ();
  isReady_ = false;

  connect (rc, static_cast<void (RunControl::*)(
                             RunControl *, const QString &,
                             Utils::OutputFormat)>(&RunControl::appendMessage),
           this, &DockedOutputWidget::appendMessage, Qt::UniqueConnection);
  connect (rc, &RunControl::finished, this, &DockedOutputWidget::markReady,
           Qt::UniqueConnection);

  show ();
  return true;
}

void DockedOutputWidget::appendMessage (RunControl */*rc*/, const QString &out,
                                        Utils::OutputFormat format)
{
  output_->appendMessage (out, format);
}

void DockedOutputWidget::markReady ()
{
  isReady_ = true;
}





DockedOutputPane::DockedOutputPane ()
  : widget_ (new QMainWindow), isEnabledButton_ (new QToolButton),
  isEnabled_ (false)
{
  isEnabledButton_->setCheckable (true);
  isEnabledButton_->setChecked (isEnabled_);
  isEnabledButton_->setIcon (Core::Icons::RUN_SMALL.icon ());
  isEnabledButton_->setToolTip (tr ("Enabled"));
  connect (isEnabledButton_, &QToolButton::toggled,
           this, &DockedOutputPane::setIsEnabled);

  connect (ProjectExplorerPlugin::instance (), &ProjectExplorerPlugin::runControlStarted,
           this, &DockedOutputPane::handleRunControlStart);

  QWidget *dummy = new QWidget;
  widget_->setCentralWidget (dummy);
  dummy->setMaximumSize (5,5);
}

DockedOutputPane::~DockedOutputPane ()
{
  delete widget_;
  delete isEnabledButton_;
}

QWidget * DockedOutputPane::outputWidget (QWidget */*parent*/)
{
  return widget_;
}

QList<QWidget *> DockedOutputPane::toolBarWidgets () const
{
  return {isEnabledButton_};
}

QString DockedOutputPane::displayName () const
{
  return tr ("Application docked output");
}

int DockedOutputPane::priorityInStatusBar () const
{
  return 60;
}

void DockedOutputPane::clearContents ()
{
}

void DockedOutputPane::visibilityChanged (bool visible)
{
  widget_->setVisible (visible);
}

void DockedOutputPane::setFocus ()
{
  widget_->setFocus ();
}

bool DockedOutputPane::hasFocus () const
{
  return widget_->hasFocus ();
}

bool DockedOutputPane::canFocus () const
{
  return true;
}

bool DockedOutputPane::canNavigate () const
{
  return false;
}

bool DockedOutputPane::canNext () const
{
  return false;
}

bool DockedOutputPane::canPrevious () const
{
  return false;
}

void DockedOutputPane::goToNext ()
{
}

void DockedOutputPane::goToPrev ()
{
}

void DockedOutputPane::handleRunControlStart (ProjectExplorer::RunControl *rc)
{
  if (!isEnabled_) {
    return;
  }

  auto docks = widget_->findChildren<DockedOutputWidget *>();
  for (auto dock: docks) {
    if (dock->reuse (rc)) {
      return;
    }
  }

  static int id = 0;
  auto newDock = new DockedOutputWidget (++id, rc);

  auto defaultDockArea = Qt::TopDockWidgetArea;
  for (auto dock: docks) {
    if (widget_->dockWidgetArea (dock) == defaultDockArea) {
      widget_->tabifyDockWidget (dock, newDock);
      return;
    }
  }
  widget_->addDockWidget (defaultDockArea, newDock);
}

void DockedOutputPane::setIsEnabled (bool isEnabled)
{
  isEnabled_ = isEnabled;
}

} // namespace DockedOutput
} // namespace Internal
} // namespace QtcUtilities

#include "DockedOutput.moc"

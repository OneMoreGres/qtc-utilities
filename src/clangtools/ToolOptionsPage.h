#pragma once

#include "ToolRunner.h"

#include <coreplugin/dialogs/ioptionspage.h>

#include <QPointer>
#include <QMap>

namespace QtcUtilities {
namespace Internal {
namespace ClangTools {

class OptionsWidget;

class ToolOptionsPage : public Core::IOptionsPage
{
  Q_OBJECT

  public:
    using Settings = QList<ToolRunnerSettings>;

    ToolOptionsPage ();

    QWidget * widget () override;
    void apply () override;
    void finish () override;

    const Settings &settings () const;

  signals:
    void settingsSaved ();

  private:
    void load ();
    void save ();

    QPointer<OptionsWidget> widget_;
    Settings settings_;

    Q_DISABLE_COPY (ToolOptionsPage)
};

} // namespace ClangTools
} // namespace Internal
} // namespace QtcUtilities

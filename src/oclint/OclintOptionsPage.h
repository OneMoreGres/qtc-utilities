#pragma once

#include "OclintRunner.h"

#include <coreplugin/dialogs/ioptionspage.h>

#include <QPointer>
#include <QMap>

namespace QtcUtilities {
namespace Internal {
namespace Oclint {

class OptionsWidget;

class OclintOptionsPage : public Core::IOptionsPage
{
  Q_OBJECT

  public:
    using Settings = OclintRunnerSettings;

    OclintOptionsPage ();

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

    Q_DISABLE_COPY (OclintOptionsPage)
};

} // namespace Oclint
} // namespace Internal
} // namespace QtcUtilities

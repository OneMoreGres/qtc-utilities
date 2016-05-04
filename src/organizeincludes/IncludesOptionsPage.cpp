#include "IncludesOptionsPage.h"
#include "IncludesConstants.h"

#include <coreplugin/icore.h>

#include <QComboBox>
#include <QBoxLayout>
#include <QLabel>

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

const QString SETTINGS_GROUP = QLatin1String ("OrganizeIncludes");
const QString SETTINGS_POLICY = QLatin1String ("policy");
const QString SETTINGS_ORDER = QLatin1String ("order");


class OptionsWidget : public QWidget
{
  public:
    OptionsWidget ();

    void set (const Settings &settings);
    void get (Settings &settings) const;

  private:
    QComboBox *policyCombo_;
    QComboBox *orderCombo_;
};

OptionsWidget::OptionsWidget ()
  : policyCombo_ (new QComboBox), orderCombo_ (new QComboBox)
{
  QMap<int, QString> policies;
  policies[MinimalEntries] = tr ("Minimal entries");
  policies[MinimalDepth] = tr ("Minimal depth");
  for (int i = PolicyFirst; i <= PolicyLast; ++i) {
    policyCombo_->addItem (policies[i]);
  }

  QMap<int, QString> orders;
  orders[SpecificFirst] = tr ("Specific first");
  orders[GeneralFirst] = tr ("Generic first");
  orders[KeepCurrent] = tr ("Keep current");
  orders[Alphabetical] = tr ("Alphabetical");
  for (int i = OrderFirst; i <= OrderLast; ++i) {
    orderCombo_->addItem (orders[i]);
  }



  auto layout = new QGridLayout;

  layout->addWidget (new QLabel (tr ("Include generation policy")), 0, 0);
  layout->addWidget (policyCombo_, 1, 0);

  layout->addWidget (new QLabel (tr ("Include order")), 0, 1);
  layout->addWidget (orderCombo_, 1, 1);

  layout->addItem (new QSpacerItem (10, 10, QSizePolicy::Expanding,
                                    QSizePolicy::Expanding), 2, 0, 1, 2);
  setLayout (layout);
}

void OptionsWidget::set (const Settings &settings)
{
  policyCombo_->setCurrentIndex (settings.policy);
  orderCombo_->setCurrentIndex (settings.order);
}

void OptionsWidget::get (Settings &settings) const
{
  settings.policy = Policy (policyCombo_->currentIndex ());
  settings.order = Order (orderCombo_->currentIndex ());
}




IncludesOptionsPage::IncludesOptionsPage ()
  : widget_ (nullptr)
{
  setId (OPTIONS_PAGE_ID);
  setDisplayName (tr ("Organize Includes"));
  setCategory (OPTIONS_CATEGORY_ID);
  setDisplayCategory (tr ("Utilities"));
  setCategoryIcon (QLatin1String (OPTIONS_CATEGORY_ICON));

  load ();
}

QWidget * IncludesOptionsPage::widget ()
{
  if (!widget_) {
    widget_ = new OptionsWidget;
  }
  widget_->set (settings_);
  return widget_.data ();
}

void IncludesOptionsPage::apply ()
{
  widget_->get (settings_);
  save ();
  emit settingsSaved ();
}

void IncludesOptionsPage::finish ()
{
}

const Settings &IncludesOptionsPage::settings () const
{
  return settings_;
}

void IncludesOptionsPage::load ()
{
  QSettings &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  settings_.policy = Policy (qsettings.value (SETTINGS_POLICY, settings_.policy).toInt ());
  settings_.order = Order (qsettings.value (SETTINGS_ORDER, settings_.order).toInt ());
  qsettings.endGroup ();
}

void IncludesOptionsPage::save ()
{
  QSettings &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  qsettings.setValue (SETTINGS_POLICY, int (settings_.policy));
  qsettings.setValue (SETTINGS_ORDER, settings_.order);
  qsettings.endGroup ();
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

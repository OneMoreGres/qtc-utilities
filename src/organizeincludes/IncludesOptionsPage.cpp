#include "IncludesOptionsPage.h"
#include "IncludesConstants.h"

#include <coreplugin/icore.h>

#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

const QString SETTINGS_GROUP = QLatin1String ("OrganizeIncludes");
const QString SETTINGS_POLICY = QLatin1String ("policy");
const QString SETTINGS_ORDER = QLatin1String ("order");
const QString SETTINGS_USE_LOCATOR = QLatin1String ("useLocator");
const QString SETTINGS_ORGANIZE_ACTIONS = QLatin1String ("organizeActions");


class OptionsWidget : public QWidget
{
  public:
    OptionsWidget ();

    void set (const Settings &settings);
    void get (Settings &settings) const;

  private:
    QComboBox *policyCombo_;
    QComboBox *orderCombo_;
    QCheckBox *useLocator_;
    QCheckBox *sortCheck_;
    QCheckBox *addCheck_;
    QCheckBox *removeCheck_;
    QCheckBox *resolveheck_;
    QCheckBox *renameCheck_;
};

OptionsWidget::OptionsWidget ()
  : policyCombo_ (new QComboBox), orderCombo_ (new QComboBox),
  useLocator_ (new QCheckBox (tr ("Use Locator to find missing symbols"))),
  sortCheck_ (new QCheckBox (tr ("Sort"))),
  addCheck_ (new QCheckBox (tr ("Add missing"))),
  removeCheck_ (new QCheckBox (tr ("Remove unused"))),
  resolveheck_ (new QCheckBox (tr ("Resolve"))),
  renameCheck_ (new QCheckBox (tr ("Rename")))
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

  auto row = 0;
  layout->addWidget (new QLabel (tr ("Include policy")), row, 0);
  layout->addWidget (new QLabel (tr ("Include order")), row, 1);

  ++row;
  layout->addWidget (policyCombo_, row, 0);
  layout->addWidget (orderCombo_, row, 1);

  ++row;
  layout->addWidget (useLocator_, row, 0);

  ++row;
  layout->addWidget (new QLabel (tr ("What 'organize' means")), row, 0, 1, 2);

  ++row;
  layout->addWidget (sortCheck_, row, 0);
  layout->addWidget (addCheck_, row, 1);

  ++row;
  layout->addWidget (removeCheck_, row, 0);
  layout->addWidget (resolveheck_, row, 1);

  ++row;
  layout->addWidget (renameCheck_, row, 0);

  ++row;
  layout->addItem (new QSpacerItem (10, 10, QSizePolicy::Expanding,
                                    QSizePolicy::Expanding), row, 0, 1, 2);
  setLayout (layout);
}

void OptionsWidget::set (const Settings &settings)
{
  policyCombo_->setCurrentIndex (settings.policy);
  orderCombo_->setCurrentIndex (settings.order);
  useLocator_->setChecked (settings.useLocator);
  sortCheck_->setChecked (settings.organizeActions & Sort);
  addCheck_->setChecked (settings.organizeActions & Add);
  removeCheck_->setChecked (settings.organizeActions & Remove);
  resolveheck_->setChecked (settings.organizeActions & Resolve);
  renameCheck_->setChecked (settings.organizeActions & Rename);
}

void OptionsWidget::get (Settings &settings) const
{
  settings.policy = Policy (policyCombo_->currentIndex ());
  settings.order = Order (orderCombo_->currentIndex ());
  settings.useLocator = useLocator_->isChecked ();
  auto actions = 0;
  actions = actions | (sortCheck_->isChecked () ? Sort : 0);
  actions = actions | (addCheck_->isChecked () ? Add : 0);
  actions = actions | (removeCheck_->isChecked () ? Remove : 0);
  actions = actions | (resolveheck_->isChecked () ? Resolve : 0);
  actions = actions | (renameCheck_->isChecked () ? Rename : 0);
  settings.organizeActions = Action (actions);
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
  settings_.useLocator = qsettings.value (SETTINGS_USE_LOCATOR,
                                          settings_.useLocator).toBool ();
  settings_.organizeActions = Action (qsettings.value (SETTINGS_ORGANIZE_ACTIONS,
                                                       settings_.organizeActions).toInt ());
  qsettings.endGroup ();
}

void IncludesOptionsPage::save ()
{
  QSettings &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  qsettings.setValue (SETTINGS_POLICY, int (settings_.policy));
  qsettings.setValue (SETTINGS_ORDER, settings_.order);
  qsettings.setValue (SETTINGS_USE_LOCATOR, settings_.useLocator);
  qsettings.setValue (SETTINGS_ORGANIZE_ACTIONS, settings_.organizeActions);
  qsettings.endGroup ();
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

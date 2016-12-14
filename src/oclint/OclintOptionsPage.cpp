#include "OclintOptionsPage.h"
#include "OclintRunner.h"
#include "OclintConstants.h"

#include "../clangtools/CheckTypes.h"

#include <coreplugin/icore.h>
#include <coreplugin/variablechooser.h>

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QFileDialog>

namespace QtcUtilities {
namespace Internal {
namespace Oclint {

const QString SETTINGS_GROUP = QLatin1String ("Oclint");
const QString SETTINGS_BINARY = QLatin1String ("binary");
const QString SETTINGS_ARGUMENTS = QLatin1String ("arguments");
const QString SETTINGS_CHECKTYPES = QLatin1String ("checkTypes");


class OclintOptionsWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit OclintOptionsWidget (Core::VariableChooser *variableChooser,
                                  QWidget *parent = nullptr);
    void set (const OclintRunnerSettings &settings);
    void get (OclintRunnerSettings &settings) const;

  private slots:
    void browseBinary ();

  private:
    QLabel *title_;
    QLineEdit *binary_;
    QPushButton *browseBinary_;
    QLineEdit *arguments_;
    QCheckBox *checkOnFileAdd_;
    QCheckBox *checkOnFileChange_;
    QCheckBox *checkOnProjectBuild_;
    QCheckBox *checkOnProjectSwitch_;

    QMap<QCheckBox *, int> typeMap_;
};

OclintOptionsWidget::OclintOptionsWidget (Core::VariableChooser *variableChooser,
                                          QWidget *parent) : QWidget (parent),
  title_ (new QLabel),
  binary_ (new QLineEdit), browseBinary_ (new QPushButton (tr ("Browse..."))),
  arguments_ (new QLineEdit),
  checkOnFileAdd_ (new QCheckBox (tr ("Check on file add"))),
  checkOnFileChange_ (new QCheckBox (tr ("Check on file change"))),
  checkOnProjectBuild_ (new QCheckBox (tr ("Check on project build"))),
  checkOnProjectSwitch_ (new QCheckBox (tr ("Check on project switch")))
{
  auto layout = new QGridLayout;

  auto row = 0;
  layout->addWidget (title_, row, 0, 1, 2);
  title_->setText ("OCLint");

  ++row;
  layout->addWidget (new QLabel (tr ("Binary:")), row, 0);
  layout->addWidget (binary_, row, 1);
  layout->addWidget (browseBinary_, row, 2);

  ++row;
  layout->addWidget (new QLabel (tr ("Arguments:")), row, 0);
  layout->addWidget (arguments_, row, 1, 1, 2);

  ++row;
  layout->addWidget (checkOnFileAdd_, row, 0);
  layout->addWidget (checkOnFileChange_, row, 1, 1, 2);

  ++row;
  layout->addWidget (checkOnProjectBuild_, row, 0);
  layout->addWidget (checkOnProjectSwitch_, row, 1, 1, 2);

  setLayout (layout);

  variableChooser->addSupportedWidget (arguments_);
  connect (browseBinary_, &QPushButton::clicked, this, &OclintOptionsWidget::browseBinary);


  typeMap_.insert (checkOnFileAdd_, ClangTools::CheckOnFileAdd);
  typeMap_.insert (checkOnFileChange_, ClangTools::CheckOnFileChange);
  typeMap_.insert (checkOnProjectBuild_, ClangTools::CheckOnProjectBuild);
  typeMap_.insert (checkOnProjectSwitch_, ClangTools::CheckOnProjectSwitch);
}

void OclintOptionsWidget::set (const OclintRunnerSettings &settings)
{
  binary_->setText (settings.binary);
  arguments_->setText (settings.arguments);
  for (const auto i: typeMap_.keys ()) {
    i->setChecked (settings.checkTypes.contains (typeMap_[i]));
  }
}

void OclintOptionsWidget::get (OclintRunnerSettings &settings) const
{
  settings.binary = binary_->text ();
  settings.arguments = arguments_->text ();
  for (auto &i: settings.extensions) {
    i = i.trimmed ();
  }
  settings.checkTypes = QList<int> () << ClangTools::CheckManual;
  for (const auto i: typeMap_.keys ()) {
    if (i->isChecked ()) {
      settings.checkTypes << typeMap_[i];
    }
  }
}

void OclintOptionsWidget::browseBinary ()
{
  auto file = QFileDialog::getOpenFileName ();
  if (!file.isEmpty ()) {
    binary_->setText (file);
  }
}



class OptionsWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit OptionsWidget ();
    void set (const OclintOptionsPage::Settings &settings);
    void get (OclintOptionsPage::Settings &settings) const;

  private:
    Core::VariableChooser *variableChooser_;
    OclintOptionsWidget *toolWidget_;
};

OptionsWidget::OptionsWidget ()
  : variableChooser_ (new Core::VariableChooser (this)),
  toolWidget_ (new OclintOptionsWidget (variableChooser_, this))
{
  auto layout = new QVBoxLayout;
  layout->addWidget (toolWidget_);
  layout->addItem (new QSpacerItem (10, 10, QSizePolicy::Expanding,
                                    QSizePolicy::Expanding));
  setLayout (layout);
}

void OptionsWidget::set (const OclintOptionsPage::Settings &settings)
{
  toolWidget_->set (settings);
}

void OptionsWidget::get (OclintOptionsPage::Settings &settings) const
{
  toolWidget_->get (settings);
}




OclintOptionsPage::OclintOptionsPage ()
  : widget_ (nullptr)
{
  setId (OPTIONS_PAGE_ID);
  setDisplayName (tr ("OCLint"));
  setCategory (OPTIONS_CATEGORY_ID);
  setDisplayCategory (tr ("Utilities"));
  setCategoryIcon (Utils::Icon (OPTIONS_CATEGORY_ICON));

  load ();
}

QWidget * OclintOptionsPage::widget ()
{
  if (!widget_) {
    widget_ = new OptionsWidget;
  }
  widget_->set (settings_);
  return widget_.data ();
}

void OclintOptionsPage::apply ()
{
  widget_->get (settings_);
  save ();
  emit settingsSaved ();
}

void OclintOptionsPage::finish ()
{
}

const OclintOptionsPage::Settings &OclintOptionsPage::settings () const
{
  return settings_;
}

void OclintOptionsPage::load ()
{
  auto &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  settings_.binary = qsettings.value (SETTINGS_BINARY, "oclint").toString ();
  auto defaultCompileDatabase = QLatin1String ("%{CurrentProject:BuildPath}/compile_commands.json");
  QString defaultArgs = QLatin1String ("-p ") + defaultCompileDatabase;
  settings_.arguments = qsettings.value (SETTINGS_ARGUMENTS, defaultArgs).toString ();
  settings_.extensions = QStringList () << "cpp" << "c" << "cxx" << "c++";
  auto types = qsettings.value (SETTINGS_CHECKTYPES).toString ().split (QLatin1String (","));
  settings_.checkTypes = QList<int> () << ClangTools::CheckManual;
  for (const auto &type: types) {
    settings_.checkTypes << type.toInt ();
  }
  qsettings.endGroup ();
}

void OclintOptionsPage::save ()
{
  auto &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  qsettings.setValue (SETTINGS_BINARY, settings_.binary);
  qsettings.setValue (SETTINGS_ARGUMENTS, settings_.arguments);
  QStringList types;
  for (auto type: settings_.checkTypes) {
    types << QString::number (type);
  }
  qsettings.setValue (SETTINGS_CHECKTYPES, types.join (QLatin1String (",")));
  qsettings.endGroup ();
}

} // namespace Oclint
} // namespace Internal
} // namespace QtcUtilities

#include "OclintOptionsPage.moc"

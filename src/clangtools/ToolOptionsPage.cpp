#include "ToolOptionsPage.h"
#include "ToolRunner.h"
#include "CheckTypes.h"
#include "ClangToolsConstants.h"

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
namespace ClangTools {

const QString SETTINGS_GROUP = QLatin1String ("ClangTools");
const QString SETTINGS_NAME = QLatin1String ("name");
const QString SETTINGS_BINARY = QLatin1String ("binary");
const QString SETTINGS_ARGUMENTS = QLatin1String ("arguments");
const QString SETTINGS_EXTENSIONS = QLatin1String ("extensions");
const QString SETTINGS_CHECKTYPES = QLatin1String ("checkTypes");

const QString NAME_TIDY = QLatin1String ("Tidy");
const QString NAME_MODULARIZE = QLatin1String ("Modularize");

class ToolOptionsWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit ToolOptionsWidget (Core::VariableChooser *variableChooser);
    void set (const ToolRunnerSettings &settings);
    void get (ToolRunnerSettings &settings) const;

  private slots:
    void browseBinary ();

  private:
    QLabel *title_;
    QLineEdit *binary_;
    QPushButton *browseBinary_;
    QLineEdit *arguments_;
    QLineEdit *extensions_;
    QCheckBox *checkOnFileAdd_;
    QCheckBox *checkOnFileChange_;
    QCheckBox *checkOnProjectBuild_;
    QCheckBox *checkOnProjectSwitch_;

    QMap<QCheckBox *, int> typeMap_;
};

ToolOptionsWidget::ToolOptionsWidget (Core::VariableChooser *variableChooser) :
  title_ (new QLabel),
  binary_ (new QLineEdit), browseBinary_ (new QPushButton (tr ("Browse..."))),
  arguments_ (new QLineEdit), extensions_ (new QLineEdit),
  checkOnFileAdd_ (new QCheckBox (tr ("Check on file add"))),
  checkOnFileChange_ (new QCheckBox (tr ("Check on file change"))),
  checkOnProjectBuild_ (new QCheckBox (tr ("Check on project build"))),
  checkOnProjectSwitch_ (new QCheckBox (tr ("Check on project switch")))
{
  auto layout = new QGridLayout;

  auto row = 0;
  layout->addWidget (title_, row, 0, 1, 2);

  ++row;
  layout->addWidget (new QLabel (tr ("Binary:")), row, 0);
  layout->addWidget (binary_, row, 1);
  layout->addWidget (browseBinary_, row, 2);

  ++row;
  layout->addWidget (new QLabel (tr ("Arguments:")), row, 0);
  layout->addWidget (arguments_, row, 1, 1, 2);

  ++row;
  layout->addWidget (new QLabel (tr ("Extensions:")), row, 0);
  layout->addWidget (extensions_, row, 1, 1, 2);

  ++row;
  layout->addWidget (checkOnFileAdd_, row, 0);
  layout->addWidget (checkOnFileChange_, row, 1, 1, 2);

  ++row;
  layout->addWidget (checkOnProjectBuild_, row, 0);
  layout->addWidget (checkOnProjectSwitch_, row, 1, 1, 2);

  setLayout (layout);


  extensions_->setToolTip (tr ("Comma separated. May be empty."));

  variableChooser->addSupportedWidget (arguments_);
  connect (browseBinary_, &QPushButton::clicked, this, &ToolOptionsWidget::browseBinary);


  typeMap_.insert (checkOnFileAdd_, CheckOnFileAdd);
  typeMap_.insert (checkOnFileChange_, CheckOnFileChange);
  typeMap_.insert (checkOnProjectBuild_, CheckOnProjectBuild);
  typeMap_.insert (checkOnProjectSwitch_, CheckOnProjectSwitch);
}

void ToolOptionsWidget::set (const ToolRunnerSettings &settings)
{
  title_->setText (settings.name);
  binary_->setText (settings.binary);
  arguments_->setText (settings.arguments);
  extensions_->setText (settings.extensions.join (QLatin1String (",")));
  for (const auto i: typeMap_.keys ()) {
    i->setChecked (settings.checkTypes.contains (typeMap_[i]));
  }
}

void ToolOptionsWidget::get (ToolRunnerSettings &settings) const
{
  settings.name = title_->text ();
  settings.binary = binary_->text ();
  settings.arguments = arguments_->text ();
  settings.extensions = extensions_->text ().split (QLatin1String (","));
  for (auto &i: settings.extensions) {
    i = i.trimmed ();
  }
  settings.checkTypes = QList<int> () << CheckManual;
  for (const auto i: typeMap_.keys ()) {
    if (i->isChecked ()) {
      settings.checkTypes << typeMap_[i];
    }
  }
}

void ToolOptionsWidget::browseBinary ()
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
    void set (const ToolOptionsPage::Settings &settings);
    void get (ToolOptionsPage::Settings &settings) const;

  private:
    Core::VariableChooser *variableChooser_;
    QList<ToolOptionsWidget *> toolWidgets_;
};

OptionsWidget::OptionsWidget ()
  : variableChooser_ (new Core::VariableChooser (this))
{
  auto layout = new QVBoxLayout;
  layout->addItem (new QSpacerItem (10, 10, QSizePolicy::Expanding,
                                    QSizePolicy::Expanding));
  setLayout (layout);
}

void OptionsWidget::set (const ToolOptionsPage::Settings &settings)
{
  for (auto i = 0, end = settings.size (); i < end; ++i) {
    if (i >= toolWidgets_.size ()) {
      auto widget = new ToolOptionsWidget (variableChooser_);
      static_cast<QBoxLayout *>(layout ())->insertWidget (i, widget);
      toolWidgets_.insert (i, widget);
    }
    auto widget = toolWidgets_[i];
    widget->set (settings[i]);
  }
}

void OptionsWidget::get (ToolOptionsPage::Settings &settings) const
{
  for (auto i = 0, end = toolWidgets_.size (); i < end; ++i) {
    if (i >= settings.size ()) {
      break;
    }
    auto widget = toolWidgets_[i];
    widget->get (settings[i]);
  }
}




ToolOptionsPage::ToolOptionsPage ()
  : widget_ (nullptr)
{
  setId (OPTIONS_PAGE_ID);
  setDisplayName (tr ("Clang tools"));
  setCategory (OPTIONS_CATEGORY_ID);
  setDisplayCategory (tr ("Utilities"));
  setCategoryIcon (QLatin1String (OPTIONS_CATEGORY_ICON));

  load ();
}

QWidget * ToolOptionsPage::widget ()
{
  if (!widget_) {
    widget_ = new OptionsWidget;
  }
  widget_->set (settings_);
  return widget_.data ();
}

void ToolOptionsPage::apply ()
{
  widget_->get (settings_);
  save ();
  emit settingsSaved ();
}

void ToolOptionsPage::finish ()
{
}

const ToolOptionsPage::Settings &ToolOptionsPage::settings () const
{
  return settings_;
}

void ToolOptionsPage::load ()
{
  auto &qsettings = *(Core::ICore::settings ());
  auto loadTool =
    [&qsettings] (const QString &tool) -> ToolRunnerSettings {
      ToolRunnerSettings s;
      qsettings.beginGroup (tool);
      s.name = qsettings.value (SETTINGS_NAME, tool).toString ();
      s.binary = qsettings.value (SETTINGS_BINARY).toString ();
      auto defaultArgs = QLatin1String ("-p %{CurrentProject:BuildPath}/compile_commands.json");
      s.arguments = qsettings.value (SETTINGS_ARGUMENTS, defaultArgs).toString ();
      s.extensions = qsettings.value (SETTINGS_EXTENSIONS).toString ().split (QLatin1String (","));
      auto types = qsettings.value (SETTINGS_CHECKTYPES).toString ().split (QLatin1String (","));
      for (const auto &type: types) {
        s.checkTypes << type.toInt ();
      }
      qsettings.endGroup ();
      return s;
    };

  settings_.clear ();
  qsettings.beginGroup (SETTINGS_GROUP);
  settings_ << loadTool (NAME_TIDY);
  settings_ << loadTool (NAME_MODULARIZE);
  qsettings.endGroup ();
}

void ToolOptionsPage::save ()
{
  auto &qsettings = *(Core::ICore::settings ());
  auto saveTool =
    [&qsettings] (const ToolRunnerSettings &s) {
      qsettings.beginGroup (s.name);
      qsettings.setValue (SETTINGS_NAME, s.name);
      qsettings.setValue (SETTINGS_BINARY, s.binary);
      qsettings.setValue (SETTINGS_ARGUMENTS, s.arguments);
      qsettings.setValue (SETTINGS_EXTENSIONS, s.extensions.join (QLatin1String (",")));
      QStringList types;
      for (auto type: s.checkTypes) {
        types << QString::number (type);
      }
      qsettings.setValue (SETTINGS_CHECKTYPES, types.join (QLatin1String (",")));
      qsettings.endGroup ();
    };

  qsettings.beginGroup (SETTINGS_GROUP);
  for (const auto &i: settings_) {
    saveTool (i);
  }
  qsettings.endGroup ();
}

} // namespace ClangTools
} // namespace Internal
} // namespace QtcUtilities

#include "ToolOptionsPage.moc"

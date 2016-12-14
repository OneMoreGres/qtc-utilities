#include "CodeDiscoverConstants.h"
#include "CodeDiscoverOptionsPage.h"

#include <coreplugin/icore.h>

#include <QSignalMapper>

#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

const QString SETTINGS_GROUP = QLatin1String ("CodeDiscover");
const QString SETTINGS_JAVA = QLatin1String ("java");
const QString SETTINGS_UML = QLatin1String ("uml");
const QString SETTINGS_DOT = QLatin1String ("dot");
const QString SETTINGS_CLASSFLAGS = QLatin1String ("classFlags");


class OptionsWidget : public QWidget
{
  Q_OBJECT

  public:
    OptionsWidget ();

    void set (const Settings &settings);
    void get (Settings &settings) const;

  private slots:
    void openFileDialog (QWidget *edit);

  private:
    QLineEdit *javaEdit_;
    QPushButton *findJavaButton_;
    QLineEdit *umlEdit_;
    QPushButton *findUmlButton_;
    QLineEdit *dotEdit_;
    QPushButton *findDotButton_;

    QSignalMapper mapper_;
};

OptionsWidget::OptionsWidget () :
  javaEdit_ (new QLineEdit), findJavaButton_ (new QPushButton (QStringLiteral ("..."))),
  umlEdit_ (new QLineEdit), findUmlButton_ (new QPushButton (QStringLiteral ("..."))),
  dotEdit_ (new QLineEdit), findDotButton_ (new QPushButton (QStringLiteral ("...")))
{
  auto layout = new QGridLayout;

  auto row = 0;
  layout->addWidget (new QLabel (tr ("Java binary")), row, 0);
  layout->addWidget (javaEdit_, row, 1);
  layout->addWidget (findJavaButton_, row, 2);

  ++row;
  layout->addWidget (new QLabel (tr ("PlantUML jar")), row, 0);
  layout->addWidget (umlEdit_, row, 1);
  layout->addWidget (findUmlButton_, row, 2);

  ++row;
  layout->addWidget (new QLabel (tr ("Dot binary")), row, 0);
  layout->addWidget (dotEdit_, row, 1);
  layout->addWidget (findDotButton_, row, 2);

  ++row;
  layout->addItem (new QSpacerItem (10, 10, QSizePolicy::Expanding,
                                    QSizePolicy::Expanding), row, 0, 1, 3);
  setLayout (layout);


  auto registerMap = [this] (QPushButton *b, QLineEdit *e) {
                       mapper_.setMapping (b, e);
                       connect (b, &QPushButton::clicked, &mapper_,
                                static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
                     };
  registerMap (findJavaButton_, javaEdit_);
  registerMap (findUmlButton_, umlEdit_);
  registerMap (findDotButton_, dotEdit_);
  connect (&mapper_, static_cast<void (QSignalMapper::*)(QWidget *)>(&QSignalMapper::mapped),
           this, &OptionsWidget::openFileDialog);
}

void OptionsWidget::set (const Settings &settings)
{
  javaEdit_->setText (settings.javaBinary);
  umlEdit_->setText (settings.umlBinary);
  dotEdit_->setText (settings.dotBinary);
}

void OptionsWidget::get (Settings &settings) const
{
  settings.javaBinary = javaEdit_->text ();
  settings.umlBinary = umlEdit_->text ();
  settings.dotBinary = dotEdit_->text ();
}

void OptionsWidget::openFileDialog (QWidget *edit)
{
  auto file = QFileDialog::getOpenFileName (this);
  if (!file.isEmpty ()) {
    static_cast<QLineEdit *>(edit)->setText (file);
  }
}




CodeDiscoverOptionsPage::CodeDiscoverOptionsPage ()
  : widget_ (nullptr), classFlags_ (ShowAll)
{
  setId (OPTIONS_PAGE_ID);
  setDisplayName (tr ("Code discover"));
  setCategory (OPTIONS_CATEGORY_ID);
  setDisplayCategory (tr ("Utilities"));
  setCategoryIcon (Utils::Icon (OPTIONS_CATEGORY_ICON));

  load ();
}

QWidget * CodeDiscoverOptionsPage::widget ()
{
  if (!widget_) {
    widget_ = new OptionsWidget;
  }
  widget_->set (settings_);
  return widget_.data ();
}

void CodeDiscoverOptionsPage::apply ()
{
  widget_->get (settings_);
  save ();
  emit settingsSaved ();
}

void CodeDiscoverOptionsPage::finish ()
{
}

const Settings &CodeDiscoverOptionsPage::settings () const
{
  return settings_;
}

ClassFlags CodeDiscoverOptionsPage::classFlags () const
{
  return classFlags_;
}

void CodeDiscoverOptionsPage::setClassFlags (ClassFlags flags)
{
  classFlags_ = flags;
  save ();
}

void CodeDiscoverOptionsPage::load ()
{
  QSettings &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  settings_.javaBinary = qsettings.value (SETTINGS_JAVA, settings_.javaBinary).toString ();
  settings_.umlBinary = qsettings.value (SETTINGS_UML, settings_.umlBinary).toString ();
  settings_.dotBinary = qsettings.value (SETTINGS_DOT, settings_.dotBinary).toString ();
  classFlags_ = ClassFlags (qsettings.value (SETTINGS_CLASSFLAGS, classFlags_).toInt ());
  qsettings.endGroup ();
}

void CodeDiscoverOptionsPage::save ()
{
  QSettings &qsettings = *(Core::ICore::settings ());
  qsettings.beginGroup (SETTINGS_GROUP);
  qsettings.setValue (SETTINGS_JAVA, settings_.javaBinary);
  qsettings.setValue (SETTINGS_UML, settings_.umlBinary);
  qsettings.setValue (SETTINGS_DOT, settings_.dotBinary);
  qsettings.setValue (SETTINGS_CLASSFLAGS, classFlags_);
  qsettings.endGroup ();
}

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

#include "CodeDiscoverOptionsPage.moc"

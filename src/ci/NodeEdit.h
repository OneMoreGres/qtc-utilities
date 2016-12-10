#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>

namespace QtcUtilities {
namespace Internal {
namespace Ci {

namespace Drone {
class Settings;
}

class ModelItem;

class NodeEdit : public QDialog
{
  Q_OBJECT

  public:
    explicit NodeEdit (QWidget *parent = 0);

    void setDrone (Drone::Settings &drone);
    Drone::Settings drone () const;

    enum class Mode {
      Drone
    };
    Mode mode () const;
    void setMode (Mode mode);

    bool savePass () const;

  private:
    Mode mode_;

    QLineEdit *url_;
    QLineEdit *user_;
    QLineEdit *pass_;
    QCheckBox *savePass_;
};

} // namespace Ci
} // namespace Internal
} // namespace QtcUtilities

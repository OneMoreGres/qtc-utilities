#pragma once

#include <QPointer>

namespace ExtensionSystem {
class IPlugin;
}
namespace Core {
class IMode;
}

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

class CodeDiscoverOptionsPage;
class CodeDiscoverWindow;
class CodeDiscoverToolRunner;
class ClassDiagramGenerator;

class CodeDiscover : public QObject
{
  Q_OBJECT

  public:
    explicit CodeDiscover (ExtensionSystem::IPlugin *plugin);

  public slots:
    void showEntryClassDiagram ();

  private slots:
    void updateSettings ();
    void handleNewImage (const QPixmap &image);

  private:
    void registerActions ();

    QPointer <CodeDiscoverOptionsPage> options_;
    QPointer <CodeDiscoverWindow> window_;
    CodeDiscoverToolRunner *runner_;
    ClassDiagramGenerator *classGenerator_;
};

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

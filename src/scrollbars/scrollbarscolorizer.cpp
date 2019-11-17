#include "scrollbarscolorizer.h"

#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditor.h>

#include <QScrollBar>
#include <QDebug>

using namespace Core;

namespace QtcUtilities {
  namespace Internal {
    namespace ScrollBarsColorizer {

      ScrollBarsColorizer::ScrollBarsColorizer (
        ExtensionSystem::IPlugin *plugin) {
        connect (EditorManager::instance (), &EditorManager::editorOpened,
                 this, &ScrollBarsColorizer::adjustColors);
      }

      void ScrollBarsColorizer::adjustColors (IEditor *editor) {
        if (!editor || !editor->widget ()) {
          return;
        }
        auto casted = qobject_cast<QPlainTextEdit *>(editor->widget ());
        if (!casted) {
          return;
        }

        if (auto bar = casted->verticalScrollBar ()) {
          const auto qss = QLatin1String (R"(
QScrollBar::handle:vertical{
background: #888888;
min-height: 20px;
}
QScrollBar:vertical{
background: white;
}
)");
          bar->setStyleSheet (qss);
        }

        if (auto bar = casted->horizontalScrollBar ()) {
          const auto qss = QLatin1String (R"(
QScrollBar::handle:horizontal{
background: #888888;
min-width: 20px;
}
QScrollBar:horizontal{
background: white;
}
)");
          bar->setStyleSheet (qss);
        }
      }

    }     // namespace ScrollBarsColorizer
  }       // namespace Internal
} // namespace QtcUtilities

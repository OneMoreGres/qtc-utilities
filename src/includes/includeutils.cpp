#include "includeutils.h"
#include "includeextractor.h"
#include "includetree.h"
#include "includemodifier.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>

#include <extensionsystem/iplugin.h>

#include <cpptools/cppmodelmanager.h>
#include <cpptools/projectpart.h>
#include <cpptools/includeutils.h>

#include <cplusplus/CppDocument.h>
#include <cplusplus/Symbol.h>

#include "utils/tooltip/tooltip.h"
#include "texteditor/basehoverhandler.h"

#include "texteditor/texteditor.h"
#include "texteditor/textmark.h"
#include "texteditor/textdocument.h"
#include <utils/executeondestruction.h>

#include <QMenu>
#include <QDir>
#include <QTextBlock>

namespace QtcUtilities {
  namespace Internal {
    namespace IncludeUtils {

      namespace {
        const char MENU_ID[] = "IncludeUtils.Menu";
        const char ACTION_ORGANIZE_INCLUDES[] = "IncludeUtils.IncludeUtils";
        const char ACTION_SORT_INCLUDES[] = "IncludeUtils.SortIncludee";
        const char ACTION_ADD_INCLUDES[] = "IncludeUtils.AddIncludes";
        const char ACTION_REMOVE_INCLUDES[] = "IncludeUtils.RemoveIncludes";
        const char ACTION_RESOLVE_INCLUDES[] = "IncludeUtils.ResolveIncludes";
        const char ACTION_RENAME_INCLUDES[] = "IncludeUtils.RenameIncludes";
        const char OPTIONS_PAGE_ID[] = "IncludeUtils.OptionaPageId";
        const char OPTIONS_CATEGORY_ID[] = "QtcUtilities.CategoryId";
        const char OPTIONS_CATEGORY_ICON[] = ":/resources/section.png";
      }

      class WeightHoverHandle : public TextEditor::BaseHoverHandler {
        void identifyMatch (TextEditor::TextEditorWidget *editorWidget, int pos,
                            ReportPriority report) override {
          Utils::ExecuteOnDestruction reportPriority ([this, report]() {
            report (priority ());
          });

          auto doc = editorWidget->textDocument ();
          auto block = doc->document ()->findBlock (pos);
          //          qDebug () << block.text ()
          //                    << doc->filePath ();
          if (!block.text ().startsWith ("#include")) {
            return;
          }

          const auto line = block.firstLineNumber () + 1;

          auto model = CppTools::CppModelManager::instance ();
          auto snapshot = model->snapshot ();
          auto cppDocument = snapshot.document (doc->filePath ().toString ());
          if (!cppDocument) {
            qDebug () << "parse failed" << cppDocument;
            return;
          }

          for (const auto &inc: cppDocument->resolvedIncludes ()) {
            //            qDebug () << inc.resolvedFileName () << inc.line () << inc.unresolvedFileName () << line;
            if (inc.line () != line) {
              continue;
            }

            const auto includes = snapshot.allIncludesForDocument (inc.resolvedFileName ());

            auto own = QFileInfo (inc.resolvedFileName ()).size ();
            auto weight = own;
            for (const auto &i: includes) {
              weight += QFileInfo (i).size ();
            }

            const QString str = inc.resolvedFileName () + " =  " +
                                QString::number (own / 1024., 'f', 1) +
                                +"(" + QString::number (weight / 1024., 'f', 1) + ") Kb";
            setToolTip (str);
            break;
          }

          //          auto cppDocument = snapshot.preprocessedDocument (doc->filePath ()->contents (),
          //                                                            documentFile);

          //          const auto line = editorWidget->textAt (pos - 100, pos + 100);
          //          qDebug () << line;
          //          setToolTip (line);

          //          const auto current = Core::EditorManager::currentDocument ();
          //          auto documentFile = current->filePath ().toString ();
        }
      };

      IncludeUtils::IncludeUtils (ExtensionSystem::IPlugin *plugin) {
        using namespace Core;
        auto menu = ActionManager::createMenu (MENU_ID);
        menu->menu ()->setTitle (tr ("Includes1"));
        ActionManager::actionContainer (Core::Constants::M_TOOLS)->addMenu (menu);

        {
          auto action = new QAction (tr ("Include utils1"), this);
          connect (action, &QAction::triggered, this, &IncludeUtils::organize);
          auto command = ActionManager::registerAction (action, ACTION_ORGANIZE_INCLUDES);
          command->setDefaultKeySequence (QKeySequence (tr ("Alt+I,Alt+I")));
          menu->addAction (command);
        }

        auto factories = Core::IEditorFactory::allEditorFactories ();
        for (const auto f: factories) {
          if (auto text = qobject_cast<TextEditor::TextEditorFactory *>(f)) {
            text->addHoverHandler (new WeightHoverHandle);
          }
        }
      }

      void IncludeUtils::organize () {
        using namespace Core;
        using namespace CppTools;
        using namespace CPlusPlus;

        const auto current = EditorManager::currentDocument ();

        auto model = CppModelManager::instance ();
        auto snapshot = model->snapshot ();
        auto documentFile = current->filePath ().toString ();
        auto cppDocument = snapshot.preprocessedDocument (current->contents (),
                                                          documentFile);
        if (!cppDocument || !cppDocument->parse ()) {
          qDebug () << "parse failed";
          return;
        }
        cppDocument->check ();


        auto control = cppDocument->control ();
        if (control->symbolCount () == 0) {
          qDebug () << "no symbols";
          return;
        }
        qDebug () << "symbols count" << control->symbolCount ();
        if (!control->firstSymbol ()) {
          qDebug () << "no first symbol";
          return;
        }

        //        Symbol **end = control->lastSymbol ();
        //        uint index = 0;
        //        for (Symbol **it = control->firstSymbol (); it != end; ++it) {
        //          ++index;
        //          qDebug () << "symbol" << index
        //                    << (*it)->fileName ()
        //                    << (*it)->line ()
        //                    << (*it)->column ()
        //                    << (*it)->visibility ()
        //                    << (*it)->type ().isUnavailable ()
        //                    << (*it)->type ()->isUndefinedType ()
        //          ;
        //          //          if ((*it)->unqualifiedName () && (*it)->unqualifiedName ()->identifier ()) {
        //          //            qDebug () << "unqualifiedName"
        //          //                      << (*it)->unqualifiedName ()->identifier ()->chars ();
        //          //          }
        //          if ((*it)->name () && (*it)->name ()->identifier ()) {
        //            qDebug () << "name" << (*it)->name ()->identifier ()->chars ();
        //          }
        //          if ((*it)->identifier ()) {
        //            qDebug () << "id" << (*it)->identifier ()->chars ();
        //          }
        //          //          if (const Function *function = (*it)->asFunction ()) {
        //          //          }
        //        }

        qDebug () << "macros" << cppDocument->definedMacros ().size ();
        for (const auto &macro: cppDocument->definedMacros ()) {
          qDebug () << macro.nameToQString ();
        }

        if (cppDocument->fileName ().isEmpty ()) {
          return;
        }

        qDebug () << "macro uses" << cppDocument->macroUses ().size ();
        for (const auto &macro: cppDocument->macroUses ()) {
          qDebug () << macro.macro ().nameToQString ();
        }

        if (cppDocument->fileName ().isEmpty ()) {
          return;
        }

        //        qDebug () << "all macros" << model->definedMacros ().size ();
        //        for (const auto &macro: model->definedMacros ()) {
        //          qDebug () << macro.key << macro.value;
        //        }

        //        if (cppDocument->fileName ().isEmpty ()) {
        //          return;
        //        }

        IncludeExtractor extractor (cppDocument, snapshot, false);
        qDebug () << "current" << cppDocument->fileName ();
        IncludeTree tree (cppDocument->fileName ());
        tree.build (snapshot);
        qDebug () << "was" << tree.includes ();



        //        for (const auto &inc: cppDocument->resolvedIncludes ()) {
        //          const auto node = tree.node (inc.resolvedFileName ());
        //          //          using Diag = Document::DiagnosticMessage;
        //          //          qDebug () << "diag" << node.fileName () << node.weight ()
        //          //                    << inc.resolvedFileName ();
        //          const auto weight = tree.totalWeight (snapshot.allIncludesForDocument (cppDocument->fileName ()));
        //          const QString str =
        //            "Weight " + QString::number (weight / 1024., 'f', 1) + " Kb";
        //          qDebug () << str << node.fileName ();
        //          //                  auto mark = new TextEditor::TextMark (
        //          //                    Utils::FileName::fromString (cppDocument->fileName ()),
        //          //                    inc.line (), "INC.INCLUDES.WEIGHT");
        //          //                  mark->setLineAnnotation (str);
        //          //                  mark->setDefaultToolTip (str);
        //          //                  mark->setToolTip (str);
        //          //          cppDocument->addDiagnosticMessage ({Diag::Warning, cppDocument->fileName (),
        //          //                                              inc.line (), 0, QString::number (node.weight ())});
        //        }
        //        return;

        tree.distribute (extractor.symbols ());
        qDebug () << "distributed symbols" << tree.root ().allSymbols ().size ()
                  << "from" << extractor.symbols ().size ();

        tree.distribute (cppDocument->macroUses ());
        qDebug () << "distributed macros" << tree.root ().allMacros ()
                  << "from" << cppDocument->macroUses ().size ();

        tree.removeEmptyPaths ();
        qDebug () << "removed empty" << tree.includes ();

        tree.removeNestedPaths ();
        qDebug () << "removed nested" << tree.includes ();

        IncludeModifier modifier (cppDocument);
        modifier.queueDuplicatesRemoval ();
        modifier.queueUpdates (tree);
        modifier.executeQueue ();

        return;

        //        auto symbol = *control->firstSymbol ();
        //        uint index = 0;
        //        while (symbol) {
        //          ++index;
        //          qDebug () << "symbol" << index
        //                    << symbol->fileName ()
        //                    << symbol->line ()
        //                    << symbol->column ()
        //                    << symbol->visibility ();
        //          if (symbol->name () && symbol->name ()->identifier ()) {
        //            qDebug () << "name" << symbol->name ()->identifier ()->chars ();
        //          }
        //          if (symbol->identifier ()) {
        //            qDebug () << "id" << symbol->identifier ()->chars ();
        //          }
        //          symbol = symbol->next ();
        //        }

        //        auto first = *control->firstSymbol ();
        //        for (auto i = 0u, end = control->symbolCount (); i < end; ++i) {
        //          auto s = first + i;
        //          qDebug () << "symbol" << i;
        //          if (s->name () && s->name ()->identifier ()) {
        //            qDebug () << "name" << s->name ()->identifier ()->chars ();
        //          }
        //          if (symbol->identifier ()) {
        //            qDebug () << "id" << s->identifier ()->chars ();
        //          }
        //        }
      }
    } // namespace IncludeUtils
  } // namespace Internal
} // namespace QtcUtilities

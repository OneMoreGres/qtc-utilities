#include "IncludesExtractor.h"

#include <extensionsystem/pluginmanager.h>

using namespace Core;
using namespace CppTools;
using namespace CPlusPlus;

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

IncludesExtractor::IncludesExtractor (Document::Ptr document, const Snapshot &snapshot)
  : ASTVisitor (document->translationUnit ()),
  document_ (document), locatorFilter_ (getLocatorFilter ())
{
  expressionType_.init (document, snapshot);
}


ILocatorFilter * IncludesExtractor::getLocatorFilter () const
{
  return ExtensionSystem::PluginManager::getObject<Core::ILocatorFilter> (
    [](Core::ILocatorFilter *i) {
          return (i->id () == Core::Id ("Classes and Methods"));
        });
}


Includes IncludesExtractor::operator () ()
{
  usages_.clear ();
  if (!translationUnit () || !translationUnit ()->ast ()) {
    return usages_;
  }
  accept (translationUnit ()->ast ());
  return usages_;
}


void IncludesExtractor::addUsage (const QString &file)
{
  Include include (file);
  if (!usages_.contains (include)) {
    usages_ << include;
  }
}


Scope * IncludesExtractor::scopeAtToken (unsigned token)
{
  unsigned line = 0;
  unsigned column = 0;
  document_->translationUnit ()->getTokenStartPosition (token, &line, &column);
  return document_->scopeAt (line);
}


QString IncludesExtractor::fileNameViaLocator (const QString &name, int types)
{
  if (locatorFilter_) {
    QFutureInterface<LocatorFilterEntry> dummy;
    auto matches = locatorFilter_->matchesFor (dummy, name);

    QString overscopedName = QStringLiteral ("::") + name;
    auto match = std::find_if (matches.cbegin (), matches.cend (),
                               [&name, types, overscopedName](const LocatorFilterEntry &i) {
            auto item = i.internalData.value<IndexItem::Ptr>();
            if (!(item->type () & types)) {
              return false;
            }
            auto fullName = item->scopedSymbolName ();
            return fullName == name || fullName.endsWith (overscopedName);
          });

    if (match != matches.end ()) {
      auto item = (*match).internalData.value<IndexItem::Ptr>();
      qDebug () << "found via locator" << item->scopedSymbolName () << item->fileName ();
      return item->fileName ();
    }
  }
  return {};
}


bool IncludesExtractor::visit (NamedTypeSpecifierAST *ast)
{
  auto scope = scopeAtToken (ast->firstToken ());
  auto typeName = overview_ (ast->name->name);
  auto matches = expressionType_ (typeName.toUtf8 (), scope);
  for (const auto &i: matches) {
    if (const auto declaration = i.declaration ()) {
      if (i.declaration ()->isForwardClassDeclaration ()) {       // check if symbol is reference
        continue;
      }
      auto fileName = QString::fromUtf8 (declaration->fileName ());
      addUsage (fileName);
      qDebug () <<  " found" << typeName << "at" << fileName;
      return true;
    }
  }

  auto fileName = fileNameViaLocator (typeName, IndexItem::Enum | IndexItem::Class);
  if (!fileName.isEmpty ()) {
    addUsage (fileName);
  }

  return true;
}

bool IncludesExtractor::visit (IdExpressionAST *ast)
{
  auto scope = scopeAtToken (ast->firstToken ());
  auto callName = overview_ (ast->name->name);
  auto matches = expressionType_ (ast, document_, scope);
  for (const auto &i: matches) {
    if (const auto declaration = i.declaration ()) {
      auto fileName = QString::fromUtf8 (declaration->fileName ());
      addUsage (fileName);
      qDebug () <<  " found" << callName << "at" << fileName;
      return true;
    }
  }

  auto fileName = fileNameViaLocator (callName, IndexItem::All);
  if (!fileName.isEmpty ()) {
    addUsage (fileName);
  }

  return true;
}
} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

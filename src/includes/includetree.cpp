#include "includetree.h"

#include <cplusplus/CppDocument.h>

#include <QSet>
#include <QDebug>

IncludeTreeNode::IncludeTreeNode (const QString &fileName) :
  fileName_ (fileName) {

}

void IncludeTreeNode::expand (const CPlusPlus::Snapshot &snapshot, IncludeRegistry &registry) {
  //  qCritical () << "expand" << fileName_;
  auto document = snapshot.document (fileName_);
  if (!document) {
    // TODO parse?
    qCritical () << "no document for" << fileName_;
    QFile file (fileName_);
    weight_ = file.size ();
    return;
  }

  if (!document->isParsed ()) {
    qCritical () << "not parsed";
    document->parse ();
  }
  if (!document->utf8Source ().isEmpty ()) {
    qCritical () << "got source";
    weight_ = document->utf8Source ().size ();
  }
  else {
    QFile file (fileName_);
    weight_ = file.size ();
  }
  //  qCritical () << fileName_ << "init weight" << weight_;

  auto includes = document->includedFiles ();
  includes.removeDuplicates ();
  children_.reserve (includes.size ());

  for (const auto &include: includes) {
    if (!registry.contains (include)) {
      auto &child = registry[include];
      child.fileName_ = include;
      //      qCritical () << fileName_ << "called expand for" << include;
      child.expand (snapshot, registry);
    }

    auto child = &registry[include];
    weight_ += child->weight ();
    children_.append (child);
    //    weight_ += child->weight ();

    //    qCritical () << fileName_ << "weight with include" << include << weight_;
    //    auto child = IncludeTreeNode (include);
    //    child.expand (snapshot, registry);
    //    weight_ += child.weight ();
    //    children_.append (child);
  }
  //  std::copy (includes.cbegin (), includes.cend (), std::back_inserter (children_));
  //  std::for_each (children_.begin (), children_.end (),
  //                 [&snapshot](IncludeTreeNode &node) {
  //    node.expand (snapshot);
  //  });
}

//void IncludeTreeNode::addChild(const QString &fileName)
//{
//    if (!registry.contains (include)) {
//        auto &child = registry[include];
//        child.expand (snapshot, registry);
//    }
//}

//void IncludeTreeNode::distribute (const QSet<CPlusPlus::Symbol *> &symbols) {
//  QHash<QString, Symbols> symbolPerFile;
//  for (const auto symbol: symbols) {
//    symbolPerFile[QString::fromUtf8 (symbol->fileName ())].append (symbol);
//  }

//  distribute (symbolPerFile);
//}

//void IncludeTreeNode::appendNotDistributed (const QSet<CPlusPlus::Symbol *> &symbols,
//                                            const CPlusPlus::Snapshot &snapshot) {
//  QSet<QString> notDistributed;
//  QHash<QString, Symbols> symbolPerFile;
//  for (const auto symbol: symbols) {
//    const auto fileName = QString::fromUtf8 (symbol->fileName ());
//    symbolPerFile[fileName].append (symbol);
//    notDistributed.insert (fileName);
//  }
//  filterWithChildren (notDistributed);

//  for (const auto &file: notDistributed) {
//    auto child = IncludeTreeNode (file);
//    child.expand (snapshot);
//    weight_ += child.weight ();
//    children_.append (child);
//  }
//}

//void IncludeTreeNode::removeEmptyPaths () {

//}

void IncludeTreeNode::distribute (const QHash<QString, Symbols> &symbolsPerFile) {
  symbols_ = symbolsPerFile.value (fileName_);
  for (auto child: children_) {
    child->distribute (symbolsPerFile);
  }
}

void IncludeTreeNode::filterWithChildren (QSet<QString> &files) const {
  files.remove (fileName_);

  if (files.isEmpty ()) {
    return;
  }

  for (const auto child: children_) {
    child->filterWithChildren (files);
  }
}

IncludeTreeNode::Symbols IncludeTreeNode::symbols () const {
  return symbols_;
}

uint IncludeTreeNode::weight () const {
  return weight_;
}

const QString &IncludeTreeNode::fileName () const {
  return fileName_;
}

IncludeTreeNode::Symbols IncludeTreeNode::allSymbols () const {
  QVector<QString> used; // infinite recursion protection
  return allSymbols (used);
}

QStringList IncludeTreeNode::allMacros () const {
  QVector<QString> used; // infinite recursion protection
  return allMacros (used);
}

bool IncludeTreeNode::hasChild (const QString &fileName) const {
  for (const auto &child: children_) {
    if (child->fileName_ == fileName || child->hasChild (fileName)) {
      return true;
    }
  }
  return false;
}

IncludeTreeNode::Symbols IncludeTreeNode::allSymbols (QVector<QString> &used) const {
  if (used.contains (fileName_)) {
    return {};
  }

  used.append (fileName_);

  if (children_.isEmpty ()) {
    return symbols_;
  }

  auto result = symbols_;
  for (const auto child: children_) {
    result += child->allSymbols (used);
  }
  //  auto result = std::accumulate (children_.cbegin (), children_.cend (), symbols_,
  //                                 [](Symbols sum, const IncludeTreeNode *node) {
  //    return sum + node->allSymbols ();
  //  });
  return result;
}

QStringList IncludeTreeNode::allMacros (QVector<QString> &used) const {
  if (used.contains (fileName_)) {
    return {};
  }

  used.append (fileName_);

  if (children_.isEmpty ()) {
    return macros_;
  }

  auto result = macros_;
  for (const auto child: children_) {
    result += child->allMacros (used);
  }
  return result;
}

const QStringList &IncludeTreeNode::macros () const {
  return macros_;
}

IncludeTree::IncludeTree (const QString &fileName) :
  root_ (fileName) {

}

IncludeTreeNode IncludeTree::node (const QString &fileName) const {
  return registry_[fileName];
}

QStringList IncludeTree::includes () const {
  QStringList result;

  auto &children = root_.children_;
  result.reserve (children.size ());

  std::transform (children.cbegin (), children.cend (), std::back_inserter (result),
                  [](const IncludeTreeNode *node) {
    return node->fileName ();
  });

  return result;
}

uint IncludeTree::totalWeight (const QSet<QString> &files) const {
  auto result = 0u;
  for (const auto &i: files) {
    result += registry_[i].weight ();
  }
  return result;
}

void IncludeTree::build (const CPlusPlus::Snapshot &snapshot) {
  root_.expand (snapshot, registry_);
}

void IncludeTree::distribute (const Symbols &symbols) {
  for (const auto symbol: symbols) {
    const auto fileName = QString::fromUtf8 (symbol->fileName ());
    if (!registry_.contains (fileName)) {
      qCritical () << "not in registry" << fileName;
      continue;
    }

    auto &node = registry_[fileName];
    node.symbols_.append (symbol);
  }
}

void IncludeTree::distribute (const QList<CPlusPlus::Document::MacroUse> &macros) {
  for (const auto &macro: macros) {
    const auto fileName = macro.macro ().fileName ();
    if (!registry_.contains (fileName)) {
      qCritical () << "not in registry" << fileName
                   << "macro" << macro.macro ().nameToQString ();
      continue;
    }

    auto &node = registry_[fileName];
    node.macros_.append (macro.macro ().toString ());
  }
}

void IncludeTree::addNew (const IncludeTree::Symbols &symbols,
                          const CPlusPlus::Snapshot &snapshot) {
  for (const auto symbol: symbols) {
    const auto fileName = QString::fromUtf8 (symbol->fileName ());
    if (registry_.contains (fileName)) {
      continue;
    }
    auto &child = registry_[fileName];
    child.expand (snapshot, registry_);
    child.symbols_.append (symbol);
    root_.children_.append (&child);
    root_.weight_ += child.weight ();
  }
}

void IncludeTree::removeEmptyPaths () {
  auto &children = root_.children_;
  children.erase (std::remove_if (children.begin (), children.end (),
                                  [](const IncludeTreeNode *node) {
    return node->allSymbols ().isEmpty () && node->allMacros ().isEmpty ();
  }), children.end ());
}

void IncludeTree::removeNestedPaths () {
  using Nodes = QVector<const IncludeTreeNode *>;
  const auto allMacros = root_.allMacros ();
  QHash<const void *, Nodes> nodePerEntity;
  auto &children = root_.children_;

  const auto macroPointer = [&allMacros](const QString &macro) {
                              Q_ASSERT (allMacros.contains (macro));
                              return &allMacros[allMacros.indexOf (macro)];
                            };

  for (const auto child: children) {
    for (const auto symbol: child->allSymbols ()) {
      nodePerEntity[symbol].append (child);
    }
    for (const auto macro: child->allMacros ()) {
      nodePerEntity[macroPointer (macro)].append (child);
    }
  }

  const auto compareNodeWeight = [](const IncludeTreeNode *l, const IncludeTreeNode *r) {
                                   Q_ASSERT (l && r);
                                   return l->weight () < r->weight ();
                                 };
  const auto compareNodeCount = [](const Nodes &l, const Nodes &r) {
                                  return l.size () < r.size ();
                                };

  QVector<const IncludeTreeNode *> usedNodes;
  while (!nodePerEntity.isEmpty ()) {
    auto unique = std::find_if (nodePerEntity.cbegin (), nodePerEntity.cend (),
                                [](const QVector<const IncludeTreeNode *> &list) {
      return list.size () == 1;
    });
    if (unique != nodePerEntity.cend ()) {
      const auto uniqueNode = unique.value ().first ();
      usedNodes.append (uniqueNode);
      for (auto symbol: uniqueNode->allSymbols ()) {
        nodePerEntity.remove (symbol);
      }
      for (auto macro: uniqueNode->allMacros ()) {
        nodePerEntity.remove (macroPointer (macro));
      }
      continue;
    }

    const auto minVariable = std::min_element (nodePerEntity.cbegin (), nodePerEntity.cend (),
                                               compareNodeCount);
    Q_ASSERT (minVariable != nodePerEntity.cend ());

    const auto symbolNodes = minVariable.value ();
    Q_ASSERT (!symbolNodes.isEmpty ());

    const auto min = std::min_element (symbolNodes.cbegin (), symbolNodes.cend (),
                                       compareNodeWeight);
    Q_ASSERT (min != symbolNodes.cend ());

    const auto minNode = *min;
    usedNodes.append (minNode);
    for (auto symbol: minNode->allSymbols ()) {
      nodePerEntity.remove (symbol);
    }
    for (auto macro: minNode->allMacros ()) {
      nodePerEntity.remove (macroPointer (macro));
    }
  }

  children.erase (std::remove_if (children.begin (), children.end (),
                                  [usedNodes](const IncludeTreeNode *node) {
    return !usedNodes.contains (node);
  }), children.end ());
}

//void IncludeTree::appendNotDistributed (const QSet<CPlusPlus::Symbol *> &symbols,
//                                        const CPlusPlus::Snapshot &snapshot) {
//  QHash<QString, IncludeTreeNode::Symbols> symbolPerFile;
//  for (const auto symbol: symbols) {
//    symbolPerFile[QString::fromUtf8 (symbol->fileName ())].append (symbol);
//  }
//}

const IncludeTreeNode &IncludeTree::root () const {
  return root_;
}

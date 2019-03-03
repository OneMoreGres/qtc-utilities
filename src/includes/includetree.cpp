#include "includetree.h"

#include <cplusplus/CppDocument.h>

#include <QSet>
#include <QDebug>

IncludeTreeNode::IncludeTreeNode (const QString &fileName) :
  fileName_ (fileName) {

}

void IncludeTreeNode::expand (const CPlusPlus::Snapshot &snapshot, IncludeRegistry &registry) {
  auto document = snapshot.document (fileName_);
  if (!document) {
    // TODO parse?
    qDebug () << "no document for" << fileName_;
    QFile file (fileName_);
    weight_ = file.size ();
  }

  weight_ = document->utf8Source ().size ();

  auto includes = document->includedFiles ();
  includes.removeDuplicates ();
  children_.reserve (includes.size ());

  for (const auto &include: includes) {
    if (!registry.contains (include)) {
      auto &child = registry[include];
      child.fileName_ = include;
      child.expand (snapshot, registry);
    }

    auto child = &registry[include];
    children_.append (child);
    weight_ += child->weight ();
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
  QVector<QString> used;
  return allSymbols (used);
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

IncludeTree::IncludeTree (const QString &fileName) :
  root_ (fileName) {

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

void IncludeTree::build (const CPlusPlus::Snapshot &snapshot) {
  root_.expand (snapshot, registry_);
}

void IncludeTree::distribute (const Symbols &symbols) {
  for (const auto symbol: symbols) {
    const auto fileName = QString::fromUtf8 (symbol->fileName ());
    if (!registry_.contains (fileName)) {
      qDebug () << "not in registry" << fileName;
      continue;
    }

    auto &node = registry_[fileName];
    node.symbols_.append (symbol);
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
    return node->allSymbols ().isEmpty ();
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

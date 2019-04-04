#pragma once

#include <QVector>
#include <QHash>

#include <cplusplus/CppDocument.h>

namespace CPlusPlus {
  class Symbol;
  class Snapshot;
}
class IncludeTreeNode;
using IncludeRegistry = QHash<QString, IncludeTreeNode>;

class IncludeTreeNode {
  public:
    using Symbols = QVector<CPlusPlus::Symbol *>;
    IncludeTreeNode (const QString &fileName = {
  });
    //    void distribute (const QSet<CPlusPlus::Symbol *> &symbols);

    uint weight () const;
    const QString &fileName () const;
    Symbols allSymbols () const;
    QStringList allMacros () const;
    bool hasChild (const QString &fileName) const;

    Symbols symbols () const;
    const QStringList &macros () const;

  private:
    friend class IncludeTree;
    //    void addChild (const QString &fileName);
    void expand (const CPlusPlus::Snapshot &snapshot,
                 IncludeRegistry &registry);
    void distribute (const QHash<QString, Symbols > &symbolsPerFile);
    void filterWithChildren (QSet<QString> &files) const;
    Symbols allSymbols (QVector<QString> &used) const;
    QStringList allMacros (QVector<QString> &used) const;

    uint weight_ = 0u;
    QString fileName_;
    Symbols symbols_;
    QStringList macros_;
    QVector<IncludeTreeNode *> children_;
};

class IncludeTree {
  public:
    using Symbols = QSet<CPlusPlus::Symbol *>;
    explicit IncludeTree (const QString &fileName);

    IncludeTreeNode node (const QString &fileName) const;
    QStringList includes () const;
    uint totalWeight (const QSet<QString> &files) const;

    void build (const CPlusPlus::Snapshot &snapshot);
    void distribute (const Symbols &symbols);
    void distribute (const QList<CPlusPlus::Document::MacroUse> &macros);
    void addNew (const Symbols &symbols, const CPlusPlus::Snapshot &snapshot);
    void removeEmptyPaths ();
    void removeNestedPaths ();

    const IncludeTreeNode &root () const;

  private:
    IncludeRegistry registry_;
    IncludeTreeNode root_;
};


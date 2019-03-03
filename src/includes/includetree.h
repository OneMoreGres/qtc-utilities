#pragma once

#include <QVector>
#include <QHash>

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

    Symbols symbols () const;

  private:
    friend class IncludeTree;
    //    void addChild (const QString &fileName);
    void expand (const CPlusPlus::Snapshot &snapshot,
                 IncludeRegistry &registry);
    void distribute (const QHash<QString, Symbols > &symbolsPerFile);
    void filterWithChildren (QSet<QString> &files) const;
    Symbols allSymbols (QVector<QString> &used) const;

    uint weight_ = 0u;
    QString fileName_;
    Symbols symbols_;
    QVector<IncludeTreeNode *> children_;
};

class IncludeTree {
  public:
    using Symbols = QSet<CPlusPlus::Symbol *>;
    explicit IncludeTree (const QString &fileName);

    QStringList includes () const;

    void build (const CPlusPlus::Snapshot &snapshot);
    void distribute (const Symbols &symbols);
    void addNew (const Symbols &symbols, const CPlusPlus::Snapshot &snapshot);
    void removeEmptyPaths ();

    const IncludeTreeNode &root () const;

  private:
    IncludeRegistry registry_;
    IncludeTreeNode root_;
};


#pragma once

#include <cplusplus/CppDocument.h>

#include <QDebug>

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

struct Include
{
  explicit Include (const CPlusPlus::Document::Include &source);
  Include (const QString &file = {}, const QString &include = {}, int line = -1,
           bool isLocal = false);

  QString directive () const;
  bool isMoc () const;

  QString file;
  QString include;
  int line;
  int groupIndex;
  bool isLocal;
  bool isAdded;
};

inline bool operator== (const Include &lhs, const Include &rhs)
{
  return lhs.file == rhs.file;
}

inline bool operator< (const Include &lhs, const Include &rhs)
{
  return lhs.file < rhs.file;
}

inline uint qHash (const Include &key, uint seed)
{
  return qHash (key.file, seed);
}

inline QDebug operator<< (QDebug dbg, const Include &data)
{
  dbg << data.file;
  return dbg;
}

using Includes = QList<Include>;

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

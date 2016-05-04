#include "IncludeMap.h"

#include <cplusplus/CppDocument.h>

using namespace CPlusPlus;

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

Includes intersected (const QSet<QString> &lhs, const Includes &rhs)
{
  Includes result;
  for (const auto &i: rhs) {
    if (lhs.contains (i.file)) {
      result << i;
    }
  }
  return result;
}


IncludeMap::IncludeMap (const Snapshot &snapshot, const Includes &includers,
                        const Includes &includes)
{
  for (const auto &includer: includers) {
    auto interesting = snapshot.allIncludesForDocument (includer.file);
    interesting.insert (includer.file);
    includers_.insert (includer, intersected (interesting, includes));

    for (const auto &included: includers_[includer]) {
      auto &names = includes_[included];
      names <<  (includer);
    }
  }

  for (const auto &i: includes) {
    if (!includes_.contains (i)) {
      includes_.insert (i, {});
    }
  }
}

Includes IncludeMap::includes () const
{
  return includes_.keys ();
}

Includes IncludeMap::includers () const
{
  return includers_.keys ();
}

void IncludeMap::organize (Policy policy)
{
  //TODO do not include internal headers
  switch (policy) {
    case MinimalDepth:
      for (const auto &i: includes ()) {
        removeIncluder (i);
      }
      break;

    case MinimalEntries:
      for (const auto &i: exclusiveIncluders ()) {
        removeIncluder (i);
      }
      while (true) {
        auto biggest = biggestIncluder ();
        if (biggest.file.isEmpty ()) {
          break;
        }
        removeIncluder (biggest);
      }
      break;

    default:
      qCritical () << "Unhandled policy switch";
      break;
  }
}

Includes IncludeMap::exclusiveIncluders () const
{
  Includes result;
  for (const auto &i: includes_.keys ()) {
    auto includers = includersOf (i);
    if (includers.size () == 1) {
      result <<  (*includers.begin ());
    }
  }
  return result;
}

Include IncludeMap::biggestIncluder () const
{
  auto keys = includers_.keys ();
  if (keys.isEmpty ()) {
    return {};
  }
  auto biggest = std::max_element (keys.cbegin (), keys.cend (),
                                   [this](const Include &l, const Include &r) {
          return includers_[l].size () < includers_[r].size ();
        });
  if (includers_[*biggest].isEmpty ()) {
    return {};
  }
  return *biggest;
}

Includes IncludeMap::includersOf (const Include &file) const
{
  return includes_.value (file);
}

void IncludeMap::removeIncluder (const Include &file)
{
  for (const auto &i: includers_.value (file)) {
    for (const auto &ii: includes_[i]) {
      includers_[ii].removeAll (i);
    }
    includes_.remove (i);
  }
  includers_.remove (file);
}


} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

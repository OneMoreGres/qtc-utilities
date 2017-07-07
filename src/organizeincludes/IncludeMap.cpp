#include "IncludeMap.h"

#include <cplusplus/CppDocument.h>

using namespace CPlusPlus;
using namespace Utils;

namespace QtcUtilities {
  namespace Internal {
    namespace OrganizeIncludes {

      bool isHeader (const QString &file) {
        return (file.endsWith (QStringLiteral (".h"))
                || file.endsWith (QStringLiteral (".hpp")));
      }

      Includes intersected (const QSet<QString> &lhs, const Includes &rhs) {
        Includes result;
        for (const auto &i: rhs) {
          if (lhs.contains (i.file)) {
            result << i;
          }
        }
        return result;
      }


      IncludeMap::IncludeMap (const Snapshot &snapshot, const Includes &includers,
                              const Includes &includes, Policy policy)
        : policy_ (policy), snapshot_ (snapshot) {
        for (const auto &includer: includers) {
          auto dependencies = snapshot.allIncludesForDocument (includer.file);
          dependencies.insert (includer.file);
          includers_.insertMulti (includer, intersected (dependencies, includes));

          for (const auto &included: includers_[includer]) {
            if (!includers_[includer].contains (included)) {
              continue;
            }
            auto &names = includes_[included];
            names <<  (includer);
          }
        }

        for (const auto &i: includes) {
          if (!includes_.contains (i) && isHeader ( (i.file))) {
            includes_.insert (i, {});
          }
        }
      }

      Includes IncludeMap::includes () const {
        return includes_.keys ();
      }

      Includes IncludeMap::includers () const {
        return includers_.keys ();
      }

      void IncludeMap::organize () {
        switch (policy_) {
          case MinimalDepth:
            for (const auto &i: includes ()) {
              removeIncluder (i);
            }
            break;

          case MinimalEntries:
            {
              auto uniting = findUnitingIncludes ();
              for (const auto &i: uniting) {
                if (!includes_.contains (i)) {
                  includes_.insert (i, {});
                }
              }
              for (const auto &i: includes ()) {
                removeIncluder (i);
                if (!uniting.contains (i)) {
                  includes_.remove (i);
                }
              }
            }
            break;

          default:
            qCritical () << "Unhandled policy switch";
            break;
        }
      }

      void IncludeMap::removeUsed () {
        QMap<Policy, ExtremumType> types = {{MinimalDepth, Smallest},
                                            {MinimalEntries, Biggest}};
        for (const auto &i: exclusiveIncluders ()) {
          removeIncluder (i);
        }
        while (true) {
          auto best = extremumEncluder (types.value (policy_, Biggest));
          if (best.file.isEmpty ()) {
            break;
          }
          removeIncluder (best);
        }

        includes_.clear ();
      }

      void IncludeMap::addMissing () {
        for (const auto &i: includers_.keys ()) {
          removeIncluder (i);
        }
        includers_.clear ();

        if (policy_ == MinimalEntries) {
          auto uniting = findUnitingIncludes ();
          includes_.clear ();
          for (const auto &i: uniting) {
            includes_.insert (i, {});
          }
        }
      }

      Includes IncludeMap::exclusiveIncluders () const {
        Includes result;
        for (const auto &i: includes_.keys ()) {
          auto includers = includersOf (i);
          if (includers.size () == 1) {
            result <<  (*includers.begin ());
          }
        }
        return result;
      }

      Include IncludeMap::extremumEncluder (IncludeMap::ExtremumType type) const {
        auto keys = includers_.keys ();
        if (keys.isEmpty ()) {
          return {};
        }
        auto best = std::max_element (keys.cbegin (), keys.cend (),
                                      [this, type](const Include &l, const Include &r) {
          return type == Biggest
          ? includers_[l].size () < includers_[r].size ()
          : includers_[l].size () > includers_[r].size ();
        });
        if (includers_[*best].isEmpty ()) {
          return {};
        }
        return *best;
      }

      Includes IncludeMap::includersOf (const Include &file) const {
        return includes_.value (file);
      }

      void IncludeMap::removeIncluder (const Include &file) {
        auto keys = includers_.keys (includers_.value (file)); // for duplicates detection
        for (const auto &i: includers_.value (file)) {
          for (const auto &ii: includes_[i]) {
            includers_[ii].removeAll (i);
          }
          includes_.remove (i);
        }

        if (includers_.remove (file) == 0) {
          return;
        }
        // keep duplicate includes except currently deleted
        keys.removeOne (file);
        for (const auto &i: keys) {
          includers_.insertMulti (i, {});
        }
      }

      Includes IncludeMap::findUnitingIncludes () const {
        using FileNames = QSet<FileName>;

        QHash<FileName, FileNames > all;
        FileNames uncovered;
        for (const auto &i: includes_.keys ()) {
          auto file = FileName::fromString (i.file);
          all[file] << file;
          uncovered << file;
          for (const auto &dependant: snapshot_.filesDependingOn (file)) {
            if (includes_.contains (dependant.toString ())) {
              all[dependant] << file << dependant;
            }
          }
        }

        auto allNames = all.keys ();
        QHash<FileName, int > weights;
        for (const auto &i: allNames) {
          weights[i] = snapshot_.includeLocationsOfDocument (i.toString ()).size ();
        }

        Includes uniting;
        while (!uncovered.isEmpty ()) {
          auto best = std::max_element (allNames.cbegin (), allNames.cend (),
                                        [&weights, &all] (const FileName &l, const FileName &r) {
            auto lSize = all[l].size ();
            auto rSize = all[r].size ();
            if (lSize == rSize) {
              return weights[l] > weights[r]; // prefer less dependencies
            }
            return lSize < rSize;
          });
          uniting << best->toString ();
          uncovered.subtract (all[*best]);
          for (auto &i: all) {
            i.intersect (uncovered);
          }
        }
        return uniting;
      }


    } // namespace OrganizeIncludes
  } // namespace Internal
} // namespace QtcUtilities

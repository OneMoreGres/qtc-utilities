#pragma once

#include "Include.h"
#include "IncludesSettings.h"

namespace CPlusPlus {
class Snapshot;
}

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

//! Tracks correspondence between files that include and files being included
class IncludeMap
{
  public:
    IncludeMap (const CPlusPlus::Snapshot &snapshot, const Includes &includers,
                const Includes &includes);

    Includes includes () const;
    Includes includers () const;
    void organize (Policy policy);

  private:
    Includes exclusiveIncluders () const;
    Include biggestIncluder () const;
    Includes includersOf (const Include &file) const;
    void removeIncluder (const Include &file);

    using Nodes = QMap<Include, Includes>;
    Nodes includers_;
    Nodes includes_;
};

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

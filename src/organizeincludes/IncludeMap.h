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
                const Includes &includes, Policy policy);

    Includes includes () const;
    Includes includers () const;
    void organize ();
    void removeUsed ();
    void addMissing ();

  private:
    enum ExtremumType { Biggest, Smallest};
    Includes exclusiveIncluders () const;
    Include extremumEncluder (ExtremumType type) const;
    Includes includersOf (const Include &file) const;
    void removeIncluder (const Include &file);
    Includes findUnitingIncludes () const;

    using Nodes = QMap<Include, Includes>;
    Nodes includers_;
    Nodes includes_;
    Policy policy_;

    const CPlusPlus::Snapshot &snapshot_;
};

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

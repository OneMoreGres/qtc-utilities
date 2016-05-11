#pragma once

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

enum Policy {
  MinimalEntries, MinimalDepth,
  PolicyFirst = MinimalEntries, PolicyLast = MinimalDepth
};

enum Order {
  SpecificFirst, GeneralFirst, KeepCurrent, Alphabetical,
  OrderFirst = SpecificFirst, OrderLast = Alphabetical
};

enum Action {
  Sort = 1 << 0,
  Add = 1 << 1,
  Remove = 1 << 2,
  Resolve = 1 << 3,
  Rename = 1 << 4,

  AllActions = Sort | Add | Remove | Resolve | Rename
};

struct Settings
{
  Policy policy {MinimalEntries};
  Order order {SpecificFirst};
  Action organizeActions {AllActions};
};

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

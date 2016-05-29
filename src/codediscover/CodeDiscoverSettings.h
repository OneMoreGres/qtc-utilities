#pragma once

#include <QString>

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

enum ClassFlags {
  ShowMembers = 1 << 0,
  ShowMethods = 1 << 1,
  ShowPublic = 1 << 2,
  ShowProtected = 1 << 3,
  ShowPrivate = 1 << 4,
  ShowBase = 1 << 5,
  ShowDerived = 1 << 6,
  ShowDependencies = 1 << 7,
  ShowDependsDetails = 1 << 8,
  ShowHierarchyDetails = 1 << 9,

  ShowAll = ShowMembers | ShowMethods | ShowPublic | ShowProtected | ShowPrivate
            | ShowBase | ShowDerived | ShowDependencies | ShowDependsDetails
            | ShowHierarchyDetails
};

struct Settings
{
  QString javaBinary;
  QString umlBinary;
  QString dotBinary;
};

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

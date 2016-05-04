#pragma once

namespace QtcUtilities {
namespace Internal {
namespace ClangTools {

enum CheckType {
  CheckOnProjectBuild, CheckOnProjectSwitch, CheckOnFileChange,
  CheckOnFileAdd, CheckManual,
  CheckTypeFirst = CheckOnProjectBuild, CheckTypeLast = CheckManual
};

} // namespace ClangTools
} // namespace Internal
} // namespace QtcUtilities

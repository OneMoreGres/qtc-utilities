#include "Include.h"

namespace QtcUtilities {
namespace Internal {
namespace OrganizeIncludes {

Include::Include (const CPlusPlus::Document::Include &source) : Include (
    source.resolvedFileName (), source.unresolvedFileName (), source.line () - 1,
    source.type () == CPlusPlus::Client::IncludeLocal)
{
}

Include::Include (const QString &file, const QString &include, int line, bool isLocal)
  : file (file), include (include), line (line), groupIndex (0), isLocal (isLocal),
  isAdded (false)
{
}

QString Include::directive () const
{
  auto result = QString (QStringLiteral ("#include %1%2%3"))
                .arg (isLocal ? '"' : '<').arg (include)
                .arg (isLocal ? '"' : '>');
  return result;
}

bool Include::isMoc () const
{
  return (include.endsWith (QStringLiteral (".moc"))
          || (include.startsWith (QStringLiteral ("moc_"))
              && include.endsWith (QStringLiteral (".cpp"))));
}

bool Include::exactMatch (const Include &rhs) const
{
  return file == rhs.file
         && include == rhs.include
         && line == rhs.line
         && groupIndex == rhs.groupIndex
         && isLocal == rhs.isLocal
         && isAdded == rhs.isAdded;
}

} // namespace OrganizeIncludes
} // namespace Internal
} // namespace QtcUtilities

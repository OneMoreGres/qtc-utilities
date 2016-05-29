#pragma once

#include "CodeDiscoverSettings.h"

#include <QObject>

namespace CPlusPlus {
class Symbol;
class Class;
}

namespace QtcUtilities {
namespace Internal {
namespace CodeDiscover {

class ClassDiagramGenerator : public QObject
{
  public:
    explicit ClassDiagramGenerator (QObject *parent = 0);

    QString generate (CPlusPlus::Symbol *symbol) const;

  private:
    ClassFlags flags_;

};

} // namespace CodeDiscover
} // namespace Internal
} // namespace QtcUtilities

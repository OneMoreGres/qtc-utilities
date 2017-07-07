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

      class ClassDiagramGenerator : public QObject {
        public:
          explicit ClassDiagramGenerator (QObject *parent = 0);

          QString generate (CPlusPlus::Symbol *symbol, ClassFlags flags) const;
      };

    } // namespace CodeDiscover
  } // namespace Internal
} // namespace QtcUtilities

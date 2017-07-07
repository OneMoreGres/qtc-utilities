#pragma once

#include <QPointer>

namespace ExtensionSystem {
  class IPlugin;
}

namespace QtcUtilities {
  namespace Internal {
    namespace OrganizeIncludes {

      class IncludesOptionsPage;

      class IncludesOrganizer : public QObject {
        Q_OBJECT

        public:
          explicit IncludesOrganizer (ExtensionSystem::IPlugin *plugin);

        public slots:
          void organize ();
          void sort ();
          void add ();
          void remove ();
          void resolve ();
          void rename ();

        private:
          void registerActions ();
          void applyActions (int actions) const;

          QPointer <IncludesOptionsPage> options_;
      };

    } // namespace OrganizeIncludes
  } // namespace Internal
} // namespace QtcUtilities

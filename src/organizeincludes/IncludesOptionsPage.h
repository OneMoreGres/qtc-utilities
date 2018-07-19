#pragma once

#include "IncludesSettings.h"

#include <coreplugin/dialogs/ioptionspage.h>

#include <QPointer>

namespace QtcUtilities {
  namespace Internal {
    namespace OrganizeIncludes {

      class OptionsWidget;

      class IncludesOptionsPage : public Core::IOptionsPage {
        Q_OBJECT

        public:
          IncludesOptionsPage (QObject *parent);
          QWidget *widget () override;
          void apply () override;
          void finish () override;

          const Settings &settings () const;

        signals:
          void settingsSaved ();

        private:
          void load ();
          void save ();

          QPointer<OptionsWidget> widget_;
          Settings settings_;

          Q_DISABLE_COPY (IncludesOptionsPage)
      };

    } // namespace OrganizeIncludes
  } // namespace Internal
} // namespace QtcUtilities

#pragma once

#include "CodeDiscoverSettings.h"

#include <coreplugin/dialogs/ioptionspage.h>

#include <QPointer>

namespace QtcUtilities {
  namespace Internal {
    namespace CodeDiscover {

      class OptionsWidget;

      class CodeDiscoverOptionsPage : public Core::IOptionsPage {
        Q_OBJECT

        public:
          CodeDiscoverOptionsPage (QObject *parent);
          QWidget *widget () override;
          void apply () override;
          void finish () override;

          const Settings &settings () const;

          ClassFlags classFlags () const;
          void setClassFlags (ClassFlags flags);

        signals:
          void settingsSaved ();

        private:
          void load ();
          void save ();

          QPointer<OptionsWidget> widget_;
          Settings settings_;
          ClassFlags classFlags_;

          Q_DISABLE_COPY (CodeDiscoverOptionsPage)
      };

    } // namespace CodeDiscover
  } // namespace Internal
} // namespace QtcUtilities

#include "gui/validator/unique_keybind_validator.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include <QKeySequence>

namespace hal
{
    UniqueKeybindValidator::UniqueKeybindValidator()
    {
        setFailText("Keybinding is already assigned by <>");
    }

    bool UniqueKeybindValidator::validate(const QString &input)
    {
        QKeySequence kseq = input;
        SettingsItemKeybind* setting =
                SettingsItemKeybind::currentKeybindAssignment(kseq);

        if(setting)
        {
            mFailText = QString("Keybinding is already assigned by <%1>")
                    .arg(setting->label());
            return false;
        }
        else
            return true;
    }
}

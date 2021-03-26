#include "gui/settings/settings_widgets/settings_widget_keybind.h"

#include "gui/label_button/label_button.h"
#include "gui/validator/unique_string_validator.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QStringList>
#include "gui/gui_globals.h"

#include "gui/settings/settings_items/settings_item_keybind.h"


namespace hal
{
    SettingsWidgetKeybind::SettingsWidgetKeybind(SettingsItemKeybind* item, QWidget* parent)
        : SettingsWidget(item, parent)
    {

        QVBoxLayout* layout = new QVBoxLayout();
        mContainer->addLayout(layout);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mKeybindEdit = new KeybindEdit(this);
        //UniqueStringValidator* v = new UniqueStringValidator(); // TODO
        //mKeybindEdit->addValidator(v);
        connect(mKeybindEdit, &KeybindEdit::editingFinished, this, &SettingsWidgetKeybind::onKeybindChanged);
        connect(mKeybindEdit, &KeybindEdit::editRejected, this, &SettingsWidgetKeybind::onKeybindEditRejected);
        layout->addWidget(mKeybindEdit);

        load(item->value());
    }

    void SettingsWidgetKeybind::load(const QVariant& value)
    {
        mKeybindEdit->setKeySequence(value.toString()); // auto-cast
    }

    QVariant SettingsWidgetKeybind::value()
    {
        QKeySequence seq = mKeybindEdit->keySequence();
        if (seq.isEmpty())
            return QVariant(QVariant::Invalid);
        return QVariant(seq); // auto-cast
    }

    void SettingsWidgetKeybind::onKeybindChanged()
    {
       this->trigger_setting_updated();
    }

    void SettingsWidgetKeybind::onKeybindEditRejected()
    {
        this->setDirty(false);
    }
}

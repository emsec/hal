#include "gui/settings/keybind_setting.h"

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

namespace hal
{
    KeybindSetting::KeybindSetting(const QString& key, const QString& title, const QString& mDescription, QWidget *parent) : SettingsWidget(key, parent)
    {
        mLabels.append(QPair<QLabel*, QString>(mName, title));

        QVBoxLayout* layout = new QVBoxLayout();
        mContainer->addLayout(layout);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mKeybindEdit = new KeybindEdit(this);
        //UniqueStringValidator* v = new UniqueStringValidator(); // TODO
        //mKeybindEdit->addValidator(v);
        connect(mKeybindEdit, &KeybindEdit::editingFinished, this, &KeybindSetting::onKeybindChanged);
        connect(mKeybindEdit, &KeybindEdit::editRejected, this, &KeybindSetting::onKeybindEditRejected);
        layout->addWidget(mKeybindEdit);

        mLabels.append(QPair<QLabel*, QString>(label, mDescription));
    }

    void KeybindSetting::load(const QVariant& value)
    {
        mKeybindEdit->setKeySequence(value.toString()); // auto-cast
    }

    QVariant KeybindSetting::value()
    {
        QKeySequence seq = mKeybindEdit->keySequence();
        if (seq.isEmpty())
            return QVariant(QVariant::Invalid);
        return QVariant(seq); // auto-cast
    }

    void KeybindSetting::onKeybindChanged()
    {
       this->triggerSettingUpdated();
    }

    void KeybindSetting::onKeybindEditRejected()
    {
        this->setDirty(false);
    }
}

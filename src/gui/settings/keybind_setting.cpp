#include "settings/keybind_setting.h"

#include "label_button/label_button.h"
#include "validator/unique_string_validator.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QStringList>
#include "gui_globals.h"

namespace hal
{
    KeybindSetting::KeybindSetting(const QString& key, const QString& title, const QString& description, QWidget *parent) : SettingsWidget(key, parent)
    {
        m_labels.append(QPair<QLabel*, QString>(m_name, title));

        QVBoxLayout* layout = new QVBoxLayout();
        m_container->addLayout(layout);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        m_KeybindEdit = new KeybindEdit(this);
        //UniqueStringValidator* v = new UniqueStringValidator(); // TODO
        //m_KeybindEdit->add_validator(v);
        connect(m_KeybindEdit, &KeybindEdit::editingFinished, this, &KeybindSetting::on_keybind_changed);
        connect(m_KeybindEdit, &KeybindEdit::edit_rejected, this, &KeybindSetting::on_KeybindEdit_rejected);
        layout->addWidget(m_KeybindEdit);

        m_labels.append(QPair<QLabel*, QString>(label, description));
    }

    void KeybindSetting::load(const QVariant& value)
    {
        m_KeybindEdit->setKeySequence(value.toString()); // auto-cast
    }

    QVariant KeybindSetting::value()
    {
        QKeySequence seq = m_KeybindEdit->keySequence();
        if (seq.isEmpty())
            return QVariant(QVariant::Invalid);
        return QVariant(seq); // auto-cast
    }

    void KeybindSetting::on_keybind_changed()
    {
       this->trigger_setting_updated();
    }

    void KeybindSetting::on_KeybindEdit_rejected()
    {
        this->set_dirty(false);
    }
}

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
    keybind_setting::keybind_setting(const QString& key, const QString& title, const QString& description, QWidget *parent) : settings_widget(key, parent)
    {
        m_labels.append(QPair<QLabel*, QString>(m_name, title));

        QVBoxLayout* layout = new QVBoxLayout();
        m_container->addLayout(layout);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        m_KeybindEdit = new KeybindEdit(this);
        //unique_string_validator* v = new unique_string_validator(); // TODO
        //m_KeybindEdit->add_validator(v);
        connect(m_KeybindEdit, &KeybindEdit::editingFinished, this, &keybind_setting::on_keybind_changed);
        connect(m_KeybindEdit, &KeybindEdit::edit_rejected, this, &keybind_setting::on_KeybindEdit_rejected);
        layout->addWidget(m_KeybindEdit);

        m_labels.append(QPair<QLabel*, QString>(label, description));
    }

    void keybind_setting::load(const QVariant& value)
    {
        m_KeybindEdit->setKeySequence(value.toString()); // auto-cast
    }

    QVariant keybind_setting::value()
    {
        QKeySequence seq = m_KeybindEdit->keySequence();
        if (seq.isEmpty())
            return QVariant(QVariant::Invalid);
        return QVariant(seq); // auto-cast
    }

    void keybind_setting::on_keybind_changed()
    {
       this->trigger_setting_updated();
    }

    void keybind_setting::on_KeybindEdit_rejected()
    {
        this->set_dirty(false);
    }
}

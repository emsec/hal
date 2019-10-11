#include "settings/keybind_setting.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QStringList>
#include "gui_globals.h"

keybind_setting::keybind_setting(const QString& key, const QString& title, const QString& description, QWidget *parent) : settings_widget(key, parent)
{
    m_labels.append(QPair<QLabel*, QString>(m_name, title));

    QVBoxLayout* layout = new QVBoxLayout();
    m_container->addLayout(layout);

    QLabel* label = new QLabel();
    layout->addWidget(label);

    m_keybind_edit = new QKeySequenceEdit(this);
    connect(m_keybind_edit, &QKeySequenceEdit::editingFinished, this, &keybind_setting::on_keybind_changed);
    layout->addWidget(m_keybind_edit);

    m_labels.append(QPair<QLabel*, QString>(label, description));
}

void keybind_setting::load(const QVariant& value)
{
    m_keybind_edit->setKeySequence(value.toString()); // auto-cast
}

QVariant keybind_setting::value()
{
    return QVariant(m_keybind_edit->keySequence()); // auto-cast
}

void keybind_setting::on_keybind_changed()
{
   this->trigger_setting_updated();
}

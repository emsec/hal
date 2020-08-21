#include "settings/spinbox_setting.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QSpinBox>
#include <QStringList>
#include "gui_globals.h"

namespace hal
{
    SpinboxSetting::SpinboxSetting(const QString& key, const QString& title, const int min, const int max, const QString& description, QWidget* parent) : SettingsWidget(key, parent)
    {
        m_labels.append(QPair<QLabel*, QString>(m_name, title));

        QHBoxLayout* layout = new QHBoxLayout();
        m_container->addLayout(layout);

        m_spinbox = new QSpinBox(this);
        m_spinbox->setMinimum(min);
        m_spinbox->setMaximum(max);
        connect(m_spinbox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SpinboxSetting::on_spinbox_value_changed);

        layout->addWidget(m_spinbox);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        m_labels.append(QPair<QLabel*, QString>(label, description));
    }

    void SpinboxSetting::load(const QVariant& value)
    {
        m_spinbox->setValue(value.toInt());
    }

    QVariant SpinboxSetting::value()
    {
        return QVariant(m_spinbox->value());
    }

    void SpinboxSetting::on_spinbox_value_changed(int value)
    {
        Q_UNUSED(value);
        trigger_setting_updated();
    }
}

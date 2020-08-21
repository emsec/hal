#include "settings/slider_setting.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QSlider>
#include <QStringList>
#include "gui_globals.h"

namespace hal
{
    SliderSetting::SliderSetting(const QString& key, const QString& title, const int min, const int max, const QString& description, QWidget* parent) : SettingsWidget(key, parent)
    {
        m_labels.append(QPair<QLabel*, QString>(m_name, title));

        QHBoxLayout* layout = new QHBoxLayout();
        m_container->addLayout(layout);

        m_slider = new QSlider(Qt::Orientation::Horizontal, this);
        m_slider->setMinimum(min);
        m_slider->setMaximum(max);
        connect(m_slider, &QSlider::valueChanged, this, &SliderSetting::on_slider_value_changed);

        layout->addWidget(m_slider);

        m_number = new QLabel(this);
        layout ->addWidget(m_number);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        m_labels.append(QPair<QLabel*, QString>(label, description));
    }

    void SliderSetting::load(const QVariant& value)
    {
        m_slider->setValue(value.toInt());
        m_number->setText(value.toString());
    }

    QVariant SliderSetting::value()
    {
        return QVariant(m_slider->value());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void SliderSetting::on_slider_value_changed()
    {
        this->trigger_setting_updated();
        m_number->setText(value().toString());
    }
}

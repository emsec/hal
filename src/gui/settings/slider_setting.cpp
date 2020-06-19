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
    slider_setting::slider_setting(const QString& key, const QString& title, const int min, const int max, const QString& description, QWidget* parent) : settings_widget(key, parent)
    {
        m_labels.append(QPair<QLabel*, QString>(m_name, title));

        QHBoxLayout* layout = new QHBoxLayout();
        m_container->addLayout(layout);

        m_slider = new QSlider(Qt::Orientation::Horizontal, this);
        m_slider->setMinimum(min);
        m_slider->setMaximum(max);
        connect(m_slider, &QSlider::valueChanged, this, &slider_setting::on_slider_value_changed);

        layout->addWidget(m_slider);

        m_number = new QLabel(this);
        layout ->addWidget(m_number);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        m_labels.append(QPair<QLabel*, QString>(label, description));
    }

    void slider_setting::load(const QVariant& value)
    {
        m_slider->setValue(value.toInt());
        m_number->setText(value.toString());
    }

    QVariant slider_setting::value()
    {
        return QVariant(m_slider->value());
    }

    // void dropdown_setting::rollback()
    // {

    // }

    void slider_setting::on_slider_value_changed()
    {
        this->trigger_setting_updated();
        m_number->setText(value().toString());
    }
}

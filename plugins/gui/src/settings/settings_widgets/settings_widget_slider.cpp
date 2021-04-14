#include "gui/settings/settings_widgets/settings_widget_slider.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QSlider>
#include <QStringList>
#include "gui/gui_globals.h"

#include "gui/settings/settings_items/settings_item_slider.h"

#include <QDebug>

namespace hal
{
    SettingsWidgetSlider::SettingsWidgetSlider(SettingsItemSlider *item, QWidget *parent)
        : SettingsWidget(item, parent)
    {

        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        mSlider = new QSlider(Qt::Orientation::Horizontal, this);
        mSlider->setMinimum(item->minimum());
        mSlider->setMaximum(item->maximum());
        connect(mSlider, &QSlider::valueChanged, this, &SettingsWidgetSlider::onSliderValueChanged);

        layout->addWidget(mSlider);

        mNumber = new QLabel(this);
        layout ->addWidget(mNumber);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        load(item->value());
    }

    void SettingsWidgetSlider::load(const QVariant& value)
    {
        mSlider->setValue(value.toInt());
        mNumber->setText(value.toString());
    }

    QVariant SettingsWidgetSlider::value()
    {
        return QVariant(mSlider->value());
    }

    void SettingsWidgetSlider::onSliderValueChanged(int value)
    {
        mNumber->setText(QString::number(value));
        trigger_setting_updated();
    }
}

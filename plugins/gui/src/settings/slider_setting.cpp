#include "gui/settings/slider_setting.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QSlider>
#include <QStringList>
#include "gui/gui_globals.h"

namespace hal
{
    SliderSetting::SliderSetting(const QString& key, const QString& title, const int min, const int max, const QString& mDescription, QWidget* parent) : SettingsWidget(key, parent)
    {
        mLabels.append(QPair<QLabel*, QString>(mName, title));

        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        mSlider = new QSlider(Qt::Orientation::Horizontal, this);
        mSlider->setMinimum(min);
        mSlider->setMaximum(max);
        connect(mSlider, &QSlider::valueChanged, this, &SliderSetting::onSliderValueChanged);

        layout->addWidget(mSlider);

        mNumber = new QLabel(this);
        layout ->addWidget(mNumber);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mLabels.append(QPair<QLabel*, QString>(label, mDescription));
    }

    void SliderSetting::load(const QVariant& value)
    {
        mSlider->setValue(value.toInt());
        mNumber->setText(value.toString());
    }

    QVariant SliderSetting::value()
    {
        return QVariant(mSlider->value());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void SliderSetting::onSliderValueChanged()
    {
        this->triggerSettingUpdated();
        mNumber->setText(value().toString());
    }
}

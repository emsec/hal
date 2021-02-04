#include "gui/settings/settings_widgets/settings_widget_spinbox.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QSpinBox>
#include <QStringList>
#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_spinbox.h"

namespace hal
{
    SettingsWidgetSpinbox::SettingsWidgetSpinbox(SettingsItemSpinbox *item, QWidget *parent)
        : SettingsWidgetNew(item, parent)
    {
        mSpinbox = new QSpinBox(this);
        mSpinbox->setMinimum(item->minimum());
        mSpinbox->setMaximum(item->maximum());
        connect(mSpinbox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SettingsWidgetSpinbox::on_spinbox_value_changed);

        mContainer->addWidget(mSpinbox);

        load(item->value());
    }

    void SettingsWidgetSpinbox::load(const QVariant& value)
    {
        mSpinbox->setValue(value.toInt());
    }

    QVariant SettingsWidgetSpinbox::value()
    {
        return QVariant(mSpinbox->value());
    }

    void SettingsWidgetSpinbox::on_spinbox_value_changed(int value)
    {
        Q_UNUSED(value);
        trigger_setting_updated();
    }
}

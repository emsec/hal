#include "gui/settings/settings_widgets/settings_widget_checkbox.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QStringList>

#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"

namespace hal
{
    SettingsWidgetCheckbox::SettingsWidgetCheckbox(SettingsItemCheckbox *item, QWidget *parent)
        : SettingsWidget(item, parent)
    {
        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        m_check_box = new QCheckBox(QString("set/unset"), this);
        connect(m_check_box, &QCheckBox::clicked, this, &SettingsWidgetCheckbox::on_state_changed);


        layout->addWidget(m_check_box);

        /*
        QLabel* label = new QLabel(item->description(),this);
        layout->addWidget(label);
        */
        load(item->value());

        //connect(this, &SettingsWidgetCheckbox::valueChanged, item, SettingsItemCheckbox::
    }

    void SettingsWidgetCheckbox::load(const QVariant& value)
    {
        m_check_box->setChecked(value.toBool());
    }

    QVariant SettingsWidgetCheckbox::value()
    {
        return QVariant(m_check_box->isChecked());
    }

    void SettingsWidgetCheckbox::on_state_changed(bool checked)
    {
        Q_UNUSED(checked);
        
        trigger_setting_updated();
    }
}

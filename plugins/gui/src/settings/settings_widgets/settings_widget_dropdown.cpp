#include "gui/settings/settings_widgets/settings_widget_dropdown.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QMap>
#include <QComboBox>
#include <QStringList>
#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"

namespace hal
{
    SettingsWidgetDropdown::SettingsWidgetDropdown(SettingsItemDropdown *item, QWidget *parent)
        : SettingsWidget(item, parent), mOptions(item->valueNames())
    {
        mComboBox = new QComboBox(this);
        mComboBox->addItems(mOptions);
        mComboBox->setStyleSheet("QComboBox{width: 150px;}");
        connect(mComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &SettingsWidgetDropdown::on_index_changed);


        mContainer->addWidget(mComboBox);


        load(item->value());
    }

    void SettingsWidgetDropdown::load(const QVariant& value)
    {
        int inx = value.toInt();
        if (inx < 0 || inx >= mOptions.size()) inx = 0;
        mComboBox->setCurrentText(mOptions.at(inx));
    }

    QVariant SettingsWidgetDropdown::value()
    {
        return mComboBox->currentIndex();
    }

    void SettingsWidgetDropdown::on_index_changed(QString text)
    {
        Q_UNUSED(text);
        this->trigger_setting_updated();
    }
}

#include "gui/settings/settings_widgets/settings_widget_dropdown.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>
#include <QDebug>
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
        connect(mComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWidgetDropdown::on_index_changed);


        mContainer->addWidget(mComboBox);
        load(item->value());
    }

    void SettingsWidgetDropdown::load(const QVariant& value)
    {
        int inx = value.toInt();
        if (inx < 0 || inx >= mOptions.size()) inx = 0;
        mComboBox->setCurrentText(mOptions.at(inx));
        QStyle* s = mComboBox->style();
        s->unpolish(mComboBox);
        s->polish(mComboBox);
    }

    QVariant SettingsWidgetDropdown::value()
    {
        return mComboBox->currentIndex();
    }

    void SettingsWidgetDropdown::on_index_changed(int inx)
    {
        Q_UNUSED(inx);
        this->trigger_setting_updated();
    }
}

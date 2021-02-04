#include "gui/settings/settings_widgets/settings_widget_text.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QComboBox>
#include <QStringList>
#include "gui/gui_globals.h"

#include "gui/settings/settings_items/settings_item_text.h"

namespace hal
{
    SettingsWidgetText::SettingsWidgetText(SettingsItemText *item, QWidget *parent) : SettingsWidgetNew(item, parent)
    {
        m_textfield = new QLineEdit(this);
        m_textfield->setPlaceholderText(QString());
        m_textfield->setStyleSheet("QLineEdit{width: 200px;}");
        connect(m_textfield, &QLineEdit::textChanged, this, &SettingsWidgetText::on_text_changed);
        mContainer->addWidget(m_textfield);



        load(item->value());
    }

    void SettingsWidgetText::load(const QVariant& value)
    {
        m_textfield->setText(value.toString());
    }

    QVariant SettingsWidgetText::value()
    {
        return QVariant(m_textfield->text());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void SettingsWidgetText::on_text_changed()
    {
        this->trigger_setting_updated();
    }
}

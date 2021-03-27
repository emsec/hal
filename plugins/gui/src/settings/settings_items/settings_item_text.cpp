#include "gui/settings/settings_items/settings_item_text.h"

#include "gui/settings/settings_manager.h"
#include "gui/settings/settings_widgets/settings_widget_text.h"

namespace hal
{
    SettingsItemText::SettingsItemText(const QString& label, const QString& tag, const QString& defVal, const QString& cat, const QString& descr, bool isGlobal)
    {
        mLabel = label;
        mTag = tag;
        mValue = defVal;
        mDefaultValue = defVal;
        mCategory = cat;
        mDescription = descr;
        mIsGlobal = isGlobal;
        
        SettingsManager::instance()->registerSetting(this);
    }

    QVariant SettingsItemText::value() const
    {
        return mValue;
    }

    QVariant SettingsItemText::defaultValue() const
    {
        return mDefaultValue;
    }

    void SettingsItemText::setValue(const QVariant& v)
    {
        QString newValue = v.toString();
        
        if(mValue == newValue)
            return;

        mValue = newValue;
        
        Q_EMIT valueChanged();
        Q_EMIT stringChanged(v.toString());
    }
    
    SettingsWidget* SettingsItemText::editWidget(QWidget* parent)
    {
        return new SettingsWidgetText(this, parent);
    }
}

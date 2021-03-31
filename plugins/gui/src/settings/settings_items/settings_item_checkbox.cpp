#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/settings/settings_manager.h"
#include "gui/settings/settings_widgets/settings_widget_checkbox.h"

namespace hal
{
    SettingsItemCheckbox::SettingsItemCheckbox(const QString& label, const QString& tag, bool defVal, const QString& cat, const QString& descr, bool isGlobal)
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

    QVariant SettingsItemCheckbox::value() const
    {
        return mValue;
    } 

    QVariant SettingsItemCheckbox::defaultValue() const
    {
        return mDefaultValue;
    }

    void SettingsItemCheckbox::setDefaultValue(const QVariant& dv)
    {
        bool newDefaultValue = dv.toBool();

        if(mDefaultValue == newDefaultValue)
            return;

        bool hasDefaultValue = (mValue == mDefaultValue);
        mDefaultValue = newDefaultValue;

        if (hasDefaultValue) setValue(mDefaultValue);
    }

    void SettingsItemCheckbox::setValue(const QVariant& v)
    {
        bool newValue = v.toBool();

        if(mValue == newValue)
            return;       

        mValue = newValue;

        Q_EMIT valueChanged();
        Q_EMIT boolChanged(mValue);
    }

    SettingsWidget* SettingsItemCheckbox::editWidget(QWidget* parent)
    {
        return new SettingsWidgetCheckbox(this, parent);
    }
}

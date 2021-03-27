#include "gui/settings/settings_items/settings_item_spinbox.h"
#include "gui/settings/settings_widgets/settings_widget_spinbox.h"
#include "gui/settings/settings_manager.h"

#include <algorithm>

namespace hal
{
    SettingsItemSpinbox::SettingsItemSpinbox(const QString& label, const QString& tag, int defVal, const QString& cat, const QString& desc, bool isGlobal)
    {
        mMinimum = 0;
        mMaximum = 100;

        mLabel = label;
        mTag = tag;
        mValue = defVal;
        mDefaultValue = defVal;
        mCategory = cat;
        mDescription = desc;
        mIsGlobal = isGlobal;

        SettingsManager::instance()->registerSetting(this);
    }

    void SettingsItemSpinbox::setRange(int min, int max)
    {
        mMinimum = min;
        mMaximum = max;
    }

    QVariant SettingsItemSpinbox::value() const
    {
        return mValue;
    }

    QVariant SettingsItemSpinbox::defaultValue() const
    {
        return mDefaultValue;
    }

    void SettingsItemSpinbox::setValue(const QVariant& v)
    {
        int newValue = v.toInt();

        if(mValue == newValue)
            return;

        mValue = newValue;
 
        Q_EMIT valueChanged();
        Q_EMIT intChanged(v.toInt());
    }

    SettingsWidget* SettingsItemSpinbox::editWidget(QWidget* parent)
    {
        return new SettingsWidgetSpinbox(this, parent);
    }
}

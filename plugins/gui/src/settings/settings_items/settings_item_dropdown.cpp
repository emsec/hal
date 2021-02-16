#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_manager_new.h"
#include "gui/settings/settings_widgets/settings_widget_dropdown.h"

namespace hal
{
    SettingsItemDropdown::SettingsItemDropdown(const QString& label, const QString& tag, int defVal, const QString& cat, const QString& desc, bool isGlobal)
    {
        mLabel = label;
        mTag = tag;
        mValue = defVal;
        mDefaultValue = defVal;
        mCategory = cat;
        mDescription = desc;
        mIsGlobal = isGlobal;

        SettingsManagerNew::instance()->registerSetting(this);
    }

    QVariant SettingsItemDropdown::value() const
    {
        return mValue;
    }

    QVariant SettingsItemDropdown::defaultValue() const
    {
        return mDefaultValue;
    }

    void SettingsItemDropdown::setValue(const QVariant& v)
    {
        int newValue = v.toInt();

        if (newValue == mValue)
            return;

        mValue = newValue;

        Q_EMIT valueChanged();
        Q_EMIT intChanged(v.toInt());
    }

    SettingsWidgetNew* SettingsItemDropdown::editWidget(QWidget* parent)
    {
        SettingsWidgetDropdown* retval = new SettingsWidgetDropdown(this, parent);
        return retval;
    }
    
    QVariant SettingsItemDropdown::persistToSettings() const
    {
        if (mValueNames.size() > mValue)
            return mValueNames.at(mValue);

        return mValue;
    }

        void SettingsItemDropdown::restoreFromSettings(const QVariant& val)
    {
        QString s = val.toString();

        if (s.isEmpty())
            return;

        if (s.at(0).isDigit())
        {
            mValue = val.toInt();

        }
        else
        {
            mValue = -1;

            for (int i=0; i<mValueNames.size(); i++)
            {
                if (s == mValueNames.at(i))
                {
                    mValue = i;
                    return;
                }
            }

        }
    }

    void SettingsItemDropdown::reloadSettings()
    {
        restoreFromSettings(SettingsManagerNew::instance()->settingsValue(this->tag()));
    }
}

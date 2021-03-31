#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/settings/settings_manager.h"
#include "gui/settings/settings_widgets/settings_widget_keybind.h"


namespace hal
{
    QMap<QKeySequence,SettingsItemKeybind*>* SettingsItemKeybind::sKeysAssigned;

    SettingsItemKeybind::SettingsItemKeybind(const QString& label, const QString& tag, const QKeySequence& defVal, const QString& cat, const QString& descr, bool isGlobal)
    {
        mLabel = label;
        mTag = tag;
        mValue = defVal;
        mDefaultValue = defVal;
        mCategory = cat;
        mDescription = descr;
        mIsGlobal = isGlobal;
        
        SettingsManager::instance()->registerSetting(this);
        registerKeybind(mValue,this);
    }

    QVariant SettingsItemKeybind::value() const
    {
        return mValue;
    }

    QVariant SettingsItemKeybind::defaultValue() const
    {
        return mDefaultValue;
    }

    void SettingsItemKeybind::registerKeybind(const QKeySequence &key, SettingsItemKeybind *setting)
    {
        if (!sKeysAssigned) sKeysAssigned = new QMap<QKeySequence,SettingsItemKeybind*>();
        sKeysAssigned->insert(key,setting);
    }

    void SettingsItemKeybind::setDefaultValue(const QVariant& dv)
    {
        QKeySequence newDefaultValue = dv.toString();

        if(mDefaultValue == newDefaultValue)
            return;

        bool hasDefaultValue = (mValue == mDefaultValue);
        mDefaultValue = newDefaultValue;

        if (hasDefaultValue) setValue(mDefaultValue);
    }

    void SettingsItemKeybind::setValue(const QVariant& v)
    {
        QKeySequence newValue = v.toString();
        
        if(mValue == newValue)
            return;

        if (sKeysAssigned)
        {
            auto it = sKeysAssigned->find(mValue);
            if (it != sKeysAssigned->end()) sKeysAssigned->erase(it);
        }
        mValue = newValue;
        registerKeybind(mValue,this);

        Q_EMIT valueChanged();
        Q_EMIT keySequenceChanged(v.toString());
    }

    SettingsWidget* SettingsItemKeybind::editWidget(QWidget* parent)
    {
        return new SettingsWidgetKeybind(this, parent);
    }

    SettingsItemKeybind *SettingsItemKeybind::currentKeybindAssignment(const QKeySequence &needle)
    {
        if (!sKeysAssigned) return nullptr;
        return sKeysAssigned->value(needle);
    }
}

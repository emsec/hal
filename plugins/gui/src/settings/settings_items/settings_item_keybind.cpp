#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/settings/settings_manager.h"
#include "gui/settings/settings_widgets/settings_widget_keybind.h"


namespace hal
{
    AssignedKeybindMap* AssignedKeybindMap::inst = nullptr;

    AssignedKeybindMap* AssignedKeybindMap::instance()
    {
        if (!inst) inst = new AssignedKeybindMap;
        return inst;
    }

    void AssignedKeybindMap::registerKeybind(const QKeySequence &key, SettingsItemKeybind *setting)
    {
        mAssignedMap.insert(key,setting);
    }

    void AssignedKeybindMap::tempAssign(const QKeySequence& newkey, SettingsItemKeybind *setting, const QKeySequence& oldkey)
    {
        if (!oldkey.isEmpty())
            mTempMap.remove(oldkey);
        mTempMap.insert(newkey,setting);
    }

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
        AssignedKeybindMap::instance()->registerKeybind(mValue,this);
    }

    QVariant SettingsItemKeybind::value() const
    {
        return mValue;
    }

    QVariant SettingsItemKeybind::defaultValue() const
    {
        return mDefaultValue;
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

        mValue = newValue;

        Q_EMIT valueChanged();
        Q_EMIT keySequenceChanged(v.toString());
    }

    SettingsWidget* SettingsItemKeybind::editWidget(QWidget* parent)
    {
        return new SettingsWidgetKeybind(this, parent);
    }
}

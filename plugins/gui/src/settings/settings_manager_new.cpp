#include "gui/settings/settings_manager_new.h"

#include <QDebug>

namespace hal
{
    SettingsManagerNew* SettingsManagerNew:: inst = nullptr;

    SettingsManagerNew* SettingsManagerNew:: instance()
    {
        if(!inst)
            inst = new SettingsManagerNew;

        return inst;
    }

    SettingsManagerNew::SettingsManagerNew(QObject* parent) : QObject(parent), mSettingsFile("emsec", "settings") {}

    void SettingsManagerNew::registerSetting(SettingsItem* item)
    {
        mSettingsList.append(item);

        if(mSettingsFile.contains(item->tag()))
            item->restoreFromSettings(mSettingsFile.value(item->tag()));

        connect(item, &SettingsItem::destroyed, this, &SettingsManagerNew::handleItemDestroyed);

        qDebug() << "new setting " << item << " " << item->tag();
    }

    void SettingsManagerNew::handleItemDestroyed(QObject* obj)
    {
        SettingsItem* item = static_cast<SettingsItem*>(obj);
        mSettingsList.removeAll(item);
    }

    void SettingsManagerNew::persistUserSettings()
    {
        for(SettingsItem* item : mSettingsList)
            mSettingsFile.setValue(item->tag(), item->value());

        mSettingsFile.sync();
    }
}

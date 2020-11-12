#include "gui/settings/settings_relay.h"
#include "gui/file_manager/file_manager.h"
#include <QDebug>

namespace hal
{
    SettingsRelay::SettingsRelay(QObject* parent) : QObject(parent)
    {
        // DEBUG print the changed setting to stdout
        connect(this, &SettingsRelay::settingChanged, this, &SettingsRelay::debug);

        // connect statement moved here since FileManager singleton might get
        // created prior to SettingsRelay
        connect(this, &SettingsRelay::settingChanged, FileManager::get_instance(), &FileManager::handleGlobalSettingChanged);
    }

    void SettingsRelay::debug(void* sender, const QString& key, const QVariant& val)
    {
        Q_UNUSED(sender);
        qDebug() << "Setting updated: " << key << " = " << val;
    }

    // void SettingsRelay::registerSender(void* sender, QString name)
    // {
    //     mSenderRegister.append(QPair<void*, QString>(sender, name));
    // }

    // void SettingsRelay::removeSender(void* sender)
    // {
    //     for (QPair<void*, QString> pair : mSenderRegister)
    //     {
    //         if (pair.first == sender)
    //             mSenderRegister.removeOne(pair);
    //     }
    // }

    void SettingsRelay::relaySettingChanged(void* sender, const QString& key, const QVariant& val)
    {
        Q_EMIT settingChanged(sender, key, val);
    }
}

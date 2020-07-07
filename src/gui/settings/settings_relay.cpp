#include "settings/settings_relay.h"
#include <QDebug>

namespace hal
{
    SettingsRelay::SettingsRelay(QObject* parent) : QObject(parent)
    {
        // DEBUG print the changed setting to stdout
        connect(this, &SettingsRelay::setting_changed, this, &SettingsRelay::debug);
    }

    void SettingsRelay::debug(void* sender, const QString& key, const QVariant& val)
    {
        Q_UNUSED(sender);
        qDebug() << "Setting updated: " << key << " = " << val;
    }

    // void SettingsRelay::register_sender(void* sender, QString name)
    // {
    //     m_sender_register.append(QPair<void*, QString>(sender, name));
    // }

    // void SettingsRelay::remove_sender(void* sender)
    // {
    //     for (QPair<void*, QString> pair : m_sender_register)
    //     {
    //         if (pair.first == sender)
    //             m_sender_register.removeOne(pair);
    //     }
    // }

    void SettingsRelay::relay_setting_changed(void* sender, const QString& key, const QVariant& val)
    {
        Q_EMIT setting_changed(sender, key, val);
    }
}

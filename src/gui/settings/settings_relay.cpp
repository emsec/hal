#include "settings/settings_relay.h"
 #include <QDebug>

settings_relay::settings_relay(QObject* parent) : QObject(parent)
{
    // DEBUG print the changed setting to stdout
    connect(this, &settings_relay::setting_changed, this, &settings_relay::debug);
}

void settings_relay::debug(void* sender, const QString& key, const QVariant& val)
{
    Q_UNUSED(sender);
    qDebug() << "Setting updated: " << key << " = " << val;
}

// void settings_relay::register_sender(void* sender, QString name)
// {
//     m_sender_register.append(QPair<void*, QString>(sender, name));
// }

// void settings_relay::remove_sender(void* sender)
// {
//     for (QPair<void*, QString> pair : m_sender_register)
//     {
//         if (pair.first == sender)
//             m_sender_register.removeOne(pair);
//     }
// }

void settings_relay::relay_setting_changed(void* sender, const QString& key, const QVariant& val)
{
    Q_EMIT setting_changed(sender, key, val);
}

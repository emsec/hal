#include "settings/settings_manager.h"

#include "core/utils.h"
#include "def.h"

#include <QSettings>

settings_manager::settings_manager(QObject* parent) : QObject(parent),
    m_settings(new QSettings(QString::fromStdString((core_utils::get_user_config_directory() / "/guisettings.ini").string()), QSettings::IniFormat)),
    m_defaults(new QSettings(QString::fromStdString((core_utils::get_user_config_directory() / "/guidefaults.ini").string()), QSettings::IniFormat))
{

}

QVariant settings_manager::get(const QString& key)
{
    return m_settings.value(key, m_defaults.value(key));
}

QVariant settings_manager::get(const QString& key, const QVariant& defaultVal)
{
    return m_settings.value(key, defaultVal);
}

void settings_manager::update(const QString& key, const QVariant& value)
{
    if (m_defaults.value(key) == value)
    {
        // if the user sets something to default, remove it from the settings file
        m_settings.remove(key);
    }
    else
    {
        m_settings.setValue(key, value);
    }
}

void settings_manager::sync()
{
    m_settings.sync();
}
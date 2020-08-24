#include "gui/settings/settings_manager.h"

#include <QDebug>
#include "hal_core/utilities/utils.h"
#include "hal_core/defines.h"
#include "gui/gui_globals.h"

#include <QSettings>

namespace hal
{
    SettingsManager::SettingsManager(QObject* parent) : QObject(parent),
        m_settings(new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guisettings.ini").string()), QSettings::IniFormat)),
        m_defaults(new QSettings(QString::fromStdString((utils::get_config_directory() / "guidefaults.ini").string()), QSettings::IniFormat))
    {
        //g_settings_relay->register_sender(this, name());
        if (m_settings->status() != QSettings::NoError) {
            qDebug() << "Failed to load guisettings.ini";
        }
        if (m_defaults->status() != QSettings::NoError) {
            qDebug() << "Failed to load guidefaults.ini";
        }
    }

    SettingsManager::~SettingsManager()
    {
        //g_settings_relay->remove_sender(this);
    }

    QVariant SettingsManager::get(const QString& key)
    {
        return this->get(key, get_default(key));
    }

    QVariant SettingsManager::get(const QString& key, const QVariant& defaultVal)
    {
        return m_settings->value(key, defaultVal);
    }

    QVariant SettingsManager::get_default(const QString& key)
    {
        return m_defaults->value(key);
    }

    QVariant SettingsManager::reset(const QString& key)
    {
        QVariant value = this->get_default(key);
        m_settings->remove(key);
        g_settings_relay->relay_setting_changed(this, key, value);
        return value;
    }

    void SettingsManager::update(const QString& key, const QVariant& value)
    {
        QVariant current = this->get(key, QVariant(QVariant::Invalid));
        if (m_defaults->value(key) == value)
        {
            // if the user sets something to default, remove it from the settings file
            m_settings->remove(key);
        }
        else
        {
            m_settings->setValue(key, value);
        }

        if (current != value)
        {
            g_settings_relay->relay_setting_changed(this, key, value);
        }
    }

    void SettingsManager::sync()
    {
        m_settings->sync();
    }
}

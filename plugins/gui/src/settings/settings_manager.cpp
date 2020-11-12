#include "gui/settings/settings_manager.h"

#include <QDebug>
#include "hal_core/utilities/utils.h"
#include "hal_core/defines.h"
#include "gui/gui_globals.h"

#include <QSettings>

namespace hal
{
    SettingsManager::SettingsManager(QObject* parent) : QObject(parent),
        mSettings(new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guisettings.ini").string()), QSettings::IniFormat)),
        mDefaults(new QSettings(QString::fromStdString((utils::get_config_directory() / "guidefaults.ini").string()), QSettings::IniFormat))
    {
        //gSettingsRelay->registerSender(this, name());
        if (mSettings->status() != QSettings::NoError) {
            qDebug() << "Failed to load guisettings.ini";
        }
        if (mDefaults->status() != QSettings::NoError) {
            qDebug() << "Failed to load guidefaults.ini";
        }
    }

    SettingsManager::~SettingsManager()
    {
        //gSettingsRelay->removeSender(this);
    }

    QVariant SettingsManager::get(const QString& key)
    {
        return this->get(key, getDefault(key));
    }

    QVariant SettingsManager::get(const QString& key, const QVariant& defaultVal)
    {
        return mSettings->value(key, defaultVal);
    }

    QVariant SettingsManager::getDefault(const QString& key)
    {
        return mDefaults->value(key);
    }

    QVariant SettingsManager::reset(const QString& key)
    {
        QVariant value = this->getDefault(key);
        mSettings->remove(key);
        gSettingsRelay->relaySettingChanged(this, key, value);
        return value;
    }

    void SettingsManager::update(const QString& key, const QVariant& value)
    {
        QVariant current = this->get(key, QVariant(QVariant::Invalid));
        if (mDefaults->value(key) == value)
        {
            // if the user sets something to default, remove it from the settings file
            mSettings->remove(key);
        }
        else
        {
            mSettings->setValue(key, value);
        }

        if (current != value)
        {
            gSettingsRelay->relaySettingChanged(this, key, value);
        }
    }

    void SettingsManager::sync()
    {
        mSettings->sync();
    }
}

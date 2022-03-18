#include "gui/settings/settings_manager.h"
#include "hal_core/utilities/utils.h"
#include <QDir>
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>

namespace hal
{
    SettingsManager* SettingsManager:: inst = nullptr;

    SettingsManager* SettingsManager:: instance()
    {
        if(!inst)
            inst = new SettingsManager;

        return inst;
    }

    SettingsManager::SettingsManager(QObject* parent)
        : QObject(parent)
    {
        QDir userConfigDir(QString::fromStdString(utils::get_user_config_directory().string()));
        QDir defaultConfigDir(QString::fromStdString(utils::get_config_directory().string()));

        mUserSettingsFile = new QSettings(userConfigDir.absoluteFilePath("guisettings.ini"), QSettings::IniFormat);
        mDefaultSettingsFile = new QSettings(defaultConfigDir.absoluteFilePath("guidefaults.ini"), QSettings::IniFormat);
    }

    void SettingsManager::registerSetting(SettingsItem* item)
    {
        mSettingsList.append(item);

        QString tag = item->tag();
        if (mDefaultSettingsFile->contains(tag))
            item->setDefaultValue(mDefaultSettingsFile->value(tag));

        if (mUserSettingsFile->contains(tag))
            item->restoreFromSettings(mUserSettingsFile->value(tag));

        connect(item, &SettingsItem::destroyed, this, &SettingsManager::handleItemDestroyed);

        // qDebug() << "new setting " << item << " " << item->tag();
    }

    void SettingsManager::handleItemDestroyed(QObject* obj)
    {
        SettingsItem* item = static_cast<SettingsItem*>(obj);
        mSettingsList.removeAll(item);
    }

    void SettingsManager::persistUserSettings()
    {
        for(SettingsItem* item : mSettingsList)
            if (item->value() != item->defaultValue())
                mUserSettingsFile->setValue(item->tag(), item->value());
            else
                mUserSettingsFile->remove(item->tag());

        mUserSettingsFile->sync();
    }

    QVariant SettingsManager::defaultValue(const QString& tag) const
    {
        return mDefaultSettingsFile->value(tag);
    }

    QVariant SettingsManager::settingsValue(const QString& tag) const
    {
        return mUserSettingsFile->value(tag);
    }

    QPoint SettingsManager::mainWindowPosition() const
    {
        return mUserSettingsFile->value("MainWindow/position", QPoint(0, 0)).toPoint();
    }

    QSize SettingsManager::mainWindowSize() const
    {
        QRect rect = QApplication::desktop()->screenGeometry();
        return mUserSettingsFile->value("MainWindow/size", rect.size()).toSize();
    }

    void SettingsManager::mainWindowSaveGeometry(const QPoint& pos, const QSize& size)
    {
        mUserSettingsFile->setValue("MainWindow/position", pos);
        mUserSettingsFile->setValue("MainWindow/size", size);
        mUserSettingsFile->sync();
    }

    LoggerSettings SettingsManager::loggerSettings() const
    {
        LoggerSettings settings;
        settings.infoSeverity = mUserSettingsFile->value("Logger/infoSeverity", QVariant(true)).toBool();
        settings.debugSeverity = mUserSettingsFile->value("Logger/debugSeverity", QVariant(false)).toBool();
        settings.warningSeverity = mUserSettingsFile->value("Logger/warningSeverity", QVariant(true)).toBool();
        settings.errorSeverity = mUserSettingsFile->value("Logger/errorSeverity", QVariant(true)).toBool();

        return settings;
    }

    void SettingsManager::saveLoggerSettings(const LoggerSettings &settings)
    {
        mUserSettingsFile->setValue("Logger/infoSeverity", settings.infoSeverity);
        mUserSettingsFile->setValue("Logger/debugSeverity", settings.debugSeverity);
        mUserSettingsFile->setValue("Logger/warningSeverity", settings.warningSeverity);
        mUserSettingsFile->setValue("Logger/errorSeverity", settings.errorSeverity);
        mUserSettingsFile->sync();
    }
}

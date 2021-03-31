#include "gui/settings/settings_manager.h"

#include <QDebug>
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
        mUserSettingsFile = QSettings("emsec", "settings");
    }

    void SettingsManager::registerSetting(SettingsItem* item)
    {
        mSettingsList.append(item);

        QString tag = item->tag();
        if (mDefaultSettingsFile.contains(tag))
            item->setDefaultValue(tag);

        if (mUserSettingsFile.contains(tag))
            item->restoreFromSettings(mUserSettingsFile.value(tag));

        connect(item, &SettingsItem::destroyed, this, &SettingsManager::handleItemDestroyed);

        qDebug() << "new setting " << item << " " << item->tag();
    }

    void SettingsManager::handleItemDestroyed(QObject* obj)
    {
        SettingsItem* item = static_cast<SettingsItem*>(obj);
        mSettingsList.removeAll(item);
    }

    void SettingsManager::persistUserSettings()
    {
        for(SettingsItem* item : mSettingsList)
            mSettingsFile.setValue(item->tag(), item->value());

        mSettingsFile.sync();
    }

    QPoint SettingsManager::mainWindowPosition() const
    {
        return mSettingsFile.value("MainWindow/position", QPoint(0, 0)).toPoint();
    }

    QSize SettingsManager::mainWindowSize() const
    {
        QRect rect = QApplication::desktop()->screenGeometry();
        return mSettingsFile.value("MainWindow/size", rect.size()).toSize();
    }

    void SettingsManager::mainWindowSaveGeometry(const QPoint& pos, const QSize& size)
    {
        mSettingsFile.setValue("MainWindow/position", pos);
        mSettingsFile.setValue("MainWindow/size", size);
        mSettingsFile.sync();
    }
}

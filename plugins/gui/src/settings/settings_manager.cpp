#include "gui/settings/settings_manager.h"
#include "hal_core/utilities/utils.h"
#include "gui/content_widget/content_widget.h"
#include "gui/content_frame/content_frame.h"
#include "gui/content_manager/content_manager.h"
#include <QDir>
#include <QApplication>
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


    void SettingsManager::saveSplitterState(const QString& tag, const QSplitter* splitter)
    {
        QString token = QString("splitter/%1").arg(tag);
        QStringList temp;
        for (int val : splitter->sizes())
            temp.append(QString::number(val));
        mUserSettingsFile->setValue(token, temp.join(','));
    }

    void SettingsManager::restoreSplitterState(const QString& tag, QSplitter* splitter) const
    {
        QString token = QString("splitter/%1").arg(tag);
        if (!mUserSettingsFile->contains(token)) return;
        QStringList temp = mUserSettingsFile->value(token).toString().split(',');
        QList<int> sizes;
        for (const QString& s : temp)
            sizes.append(s.toInt());
        splitter->setSizes(sizes);
    }

    void SettingsManager::widgetDetach(ContentWidget* cw) const
    {
        QString token    = cw->name() + "/%1";
        if (mUserSettingsFile->contains(token.arg("detachedPosition")))
        {
            cw->detach();
            ContentFrame*cf = cw->detachedFrame();
            if (cf) cf->setGeometry(QRect(mUserSettingsFile->value(token.arg("detachedPosition")).toPoint(),
                                      mUserSettingsFile->value(token.arg("size")).toSize()));
        }
    }

    ContentWidgetPlacement SettingsManager::widgetPlacement(ContentWidget *cw) const
    {
        ContentWidgetPlacement retval;
        QString token    = cw->name() + "/%1";
        retval.widget    = cw;
        retval.index     = mUserSettingsFile->value(token.arg("index"), -1).toInt();
        retval.visible   = mUserSettingsFile->value(token.arg("visible"), true).toBool();
        retval.anchorPos = ContentLayout::positionFromString(mUserSettingsFile->value(token.arg("anchor")).toString());
        if (mUserSettingsFile->contains(token.arg("size")))
            cw->setGeometry(QRect(QPoint(0,24),mUserSettingsFile->value("size").toSize()));
        if (mUserSettingsFile->contains(token.arg("detachedPosition")))
            retval.visible = false;
        return retval;
    }

    void SettingsManager::widgetsSaveGeometry(ContentLayout::Position anchorPos, QList<const ContentWidget*>& widgets)
    {
        int index = 0;
        for (const ContentWidget* cw : widgets)
        {
            QString token = cw->name() + "/%1";
            mUserSettingsFile->setValue(token.arg("anchor"), ContentLayout::positionToString(anchorPos));
            mUserSettingsFile->setValue(token.arg("index"), index++);
            mUserSettingsFile->setValue(token.arg("visible"), cw->isVisible());
            mUserSettingsFile->setValue(token.arg("size"), cw->size());
            if (cw->detachedFrame())
                mUserSettingsFile->setValue(token.arg("detachedPosition"), cw->detachedFrame()->pos());
            else
                mUserSettingsFile->remove(token.arg("detachedPosition"));
        }
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

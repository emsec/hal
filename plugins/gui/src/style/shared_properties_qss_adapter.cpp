#include "gui/style/shared_properties_qss_adapter.h"

#include <QStyle>

namespace hal
{
    SharedPropertiesQssAdapter::SharedPropertiesQssAdapter(QWidget* parent) : QWidget(parent)
    {
        //DEFAULT VALUES
        mOpenIconPath = ":/icons/folder";
        mOpenIconStyle = "all->#D3C68A";
        mSaveIconPath = ":/icons/folder-down";
        mSaveIconStyle = "all->#D3C68A";
        mScheduleIconPath = ":/icons/list";
        mScheduleIconStyle = "all->#00FF00";
        mRunIconPath = ":/icons/run";
        mRunIconStyle = "all->#00FF00";
        mContentIconPath = ":/icons/content";
        mContentIconStyle = "all->#969696";
        mSettingsIconPath = ":/icons/settings";
        mSettingsIconStyle = "all->#969696";

        repolish();
    }

    const SharedPropertiesQssAdapter* SharedPropertiesQssAdapter::instance()
    {
        static SharedPropertiesQssAdapter* instance = nullptr;

        if (!instance)
            instance = new SharedPropertiesQssAdapter();

        return instance;
    }

    void SharedPropertiesQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    QString SharedPropertiesQssAdapter::openIconPath() const
    {
        return mOpenIconPath;
    }

    QString SharedPropertiesQssAdapter::openIconStyle() const
    {
        return mOpenIconStyle;
    }

    QString SharedPropertiesQssAdapter::saveIconPath() const
    {
        return mSaveIconPath;
    }

    QString SharedPropertiesQssAdapter::saveIconStyle() const
    {
        return mSaveIconStyle;
    }

    QString SharedPropertiesQssAdapter::scheduleIconPath() const
    {
        return mScheduleIconPath;
    }

    QString SharedPropertiesQssAdapter::scheduleIconStyle() const
    {
        return mScheduleIconStyle;
    }

    QString SharedPropertiesQssAdapter::runIconPath() const
    {
        return mRunIconPath;
    }

    QString SharedPropertiesQssAdapter::runIconStyle() const
    {
        return mRunIconStyle;
    }

    QString SharedPropertiesQssAdapter::contentIconPath() const
    {
        return mContentIconPath;
    }

    QString SharedPropertiesQssAdapter::contentIconStyle() const
    {
        return mContentIconStyle;
    }

    QString SharedPropertiesQssAdapter::settingsIconPath() const
    {
        return mSettingsIconPath;
    }

    QString SharedPropertiesQssAdapter::settingsIconStyle() const
    {
        return mSettingsIconStyle;
    }

    void SharedPropertiesQssAdapter::setOpenIconPath(const QString& path)
    {
        mOpenIconPath = path;
    }

    void SharedPropertiesQssAdapter::setOpenIconStyle(const QString& style)
    {
        mOpenIconStyle = style;
    }

    void SharedPropertiesQssAdapter::setSaveIconPath(const QString& path)
    {
        mSaveIconPath = path;
    }

    void SharedPropertiesQssAdapter::setSaveIconStyle(const QString& style)
    {
        mSaveIconStyle = style;
    }

    void SharedPropertiesQssAdapter::setScheduleIconPath(const QString& path)
    {
        mScheduleIconPath = path;
    }

    void SharedPropertiesQssAdapter::setScheduleIconStyle(const QString& style)
    {
        mScheduleIconStyle = style;
    }

    void SharedPropertiesQssAdapter::setRunIconPath(const QString& path)
    {
        mRunIconPath = path;
    }

    void SharedPropertiesQssAdapter::setRunIconStyle(const QString& style)
    {
        mRunIconStyle = style;
    }

    void SharedPropertiesQssAdapter::setContentIconPath(const QString& path)
    {
        mContentIconPath = path;
    }

    void SharedPropertiesQssAdapter::setContentIconStyle(const QString& style)
    {
        mContentIconStyle = style;
    }

    void SharedPropertiesQssAdapter::setSettingsIconPath(const QString& path)
    {
        mSettingsIconPath = path;
    }

    void SharedPropertiesQssAdapter::setSettingsIconStyle(const QString& style)
    {
        mSettingsIconStyle = style;
    }
}

#include "style/shared_properties_qss_adapter.h"

#include <QStyle>

namespace hal
{
    SharedPropertiesQssAdapter::SharedPropertiesQssAdapter(QWidget* parent) : QWidget(parent)
    {
        //DEFAULT VALUES
        m_open_icon_path = ":/icons/folder";
        m_open_icon_style = "all->#D3C68A";
        m_save_icon_path = ":/icons/folder-down";
        m_save_icon_style = "all->#D3C68A";
        m_schedule_icon_path = ":/icons/list";
        m_schedule_icon_style = "all->#00FF00";
        m_run_icon_path = ":/icons/run";
        m_run_icon_style = "all->#00FF00";
        m_content_icon_path = ":/icons/content";
        m_content_icon_style = "all->#969696";
        m_settings_icon_path = ":/icons/settings";
        m_settings_icon_style = "all->#969696";

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

    QString SharedPropertiesQssAdapter::open_icon_path() const
    {
        return m_open_icon_path;
    }

    QString SharedPropertiesQssAdapter::open_icon_style() const
    {
        return m_open_icon_style;
    }

    QString SharedPropertiesQssAdapter::save_icon_path() const
    {
        return m_save_icon_path;
    }

    QString SharedPropertiesQssAdapter::save_icon_style() const
    {
        return m_save_icon_style;
    }

    QString SharedPropertiesQssAdapter::schedule_icon_path() const
    {
        return m_schedule_icon_path;
    }

    QString SharedPropertiesQssAdapter::schedule_icon_style() const
    {
        return m_schedule_icon_style;
    }

    QString SharedPropertiesQssAdapter::run_icon_path() const
    {
        return m_run_icon_path;
    }

    QString SharedPropertiesQssAdapter::run_icon_style() const
    {
        return m_run_icon_style;
    }

    QString SharedPropertiesQssAdapter::content_icon_path() const
    {
        return m_content_icon_path;
    }

    QString SharedPropertiesQssAdapter::content_icon_style() const
    {
        return m_content_icon_style;
    }

    QString SharedPropertiesQssAdapter::settings_icon_path() const
    {
        return m_settings_icon_path;
    }

    QString SharedPropertiesQssAdapter::settings_icon_style() const
    {
        return m_settings_icon_style;
    }

    void SharedPropertiesQssAdapter::set_open_icon_path(const QString& path)
    {
        m_open_icon_path = path;
    }

    void SharedPropertiesQssAdapter::set_open_icon_style(const QString& style)
    {
        m_open_icon_style = style;
    }

    void SharedPropertiesQssAdapter::set_save_icon_path(const QString& path)
    {
        m_save_icon_path = path;
    }

    void SharedPropertiesQssAdapter::set_save_icon_style(const QString& style)
    {
        m_save_icon_style = style;
    }

    void SharedPropertiesQssAdapter::set_schedule_icon_path(const QString& path)
    {
        m_schedule_icon_path = path;
    }

    void SharedPropertiesQssAdapter::set_schedule_icon_style(const QString& style)
    {
        m_schedule_icon_style = style;
    }

    void SharedPropertiesQssAdapter::set_run_icon_path(const QString& path)
    {
        m_run_icon_path = path;
    }

    void SharedPropertiesQssAdapter::set_run_icon_style(const QString& style)
    {
        m_run_icon_style = style;
    }

    void SharedPropertiesQssAdapter::set_content_icon_path(const QString& path)
    {
        m_content_icon_path = path;
    }

    void SharedPropertiesQssAdapter::set_content_icon_style(const QString& style)
    {
        m_content_icon_style = style;
    }

    void SharedPropertiesQssAdapter::set_settings_icon_path(const QString& path)
    {
        m_settings_icon_path = path;
    }

    void SharedPropertiesQssAdapter::set_settings_icon_style(const QString& style)
    {
        m_settings_icon_style = style;
    }
}

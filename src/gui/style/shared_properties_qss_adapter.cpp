#include "style/shared_properties_qss_adapter.h"

#include <QStyle>
namespace hal{
shared_properties_qss_adapter::shared_properties_qss_adapter(QWidget* parent) : QWidget(parent)
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

const shared_properties_qss_adapter* shared_properties_qss_adapter::instance()
{
    static shared_properties_qss_adapter* instance = nullptr;

    if (!instance)
        instance = new shared_properties_qss_adapter();

    return instance;
}

void shared_properties_qss_adapter::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);
}

QString shared_properties_qss_adapter::open_icon_path() const
{
    return m_open_icon_path;
}

QString shared_properties_qss_adapter::open_icon_style() const
{
    return m_open_icon_style;
}

QString shared_properties_qss_adapter::save_icon_path() const
{
    return m_save_icon_path;
}

QString shared_properties_qss_adapter::save_icon_style() const
{
    return m_save_icon_style;
}

QString shared_properties_qss_adapter::schedule_icon_path() const
{
    return m_schedule_icon_path;
}

QString shared_properties_qss_adapter::schedule_icon_style() const
{
    return m_schedule_icon_style;
}

QString shared_properties_qss_adapter::run_icon_path() const
{
    return m_run_icon_path;
}

QString shared_properties_qss_adapter::run_icon_style() const
{
    return m_run_icon_style;
}

QString shared_properties_qss_adapter::content_icon_path() const
{
    return m_content_icon_path;
}

QString shared_properties_qss_adapter::content_icon_style() const
{
    return m_content_icon_style;
}

QString shared_properties_qss_adapter::settings_icon_path() const
{
    return m_settings_icon_path;
}

QString shared_properties_qss_adapter::settings_icon_style() const
{
    return m_settings_icon_style;
}

void shared_properties_qss_adapter::set_open_icon_path(const QString& path)
{
    m_open_icon_path = path;
}

void shared_properties_qss_adapter::set_open_icon_style(const QString& style)
{
    m_open_icon_style = style;
}

void shared_properties_qss_adapter::set_save_icon_path(const QString& path)
{
    m_save_icon_path = path;
}

void shared_properties_qss_adapter::set_save_icon_style(const QString& style)
{
    m_save_icon_style = style;
}

void shared_properties_qss_adapter::set_schedule_icon_path(const QString& path)
{
    m_schedule_icon_path = path;
}

void shared_properties_qss_adapter::set_schedule_icon_style(const QString& style)
{
    m_schedule_icon_style = style;
}

void shared_properties_qss_adapter::set_run_icon_path(const QString& path)
{
    m_run_icon_path = path;
}

void shared_properties_qss_adapter::set_run_icon_style(const QString& style)
{
    m_run_icon_style = style;
}

void shared_properties_qss_adapter::set_content_icon_path(const QString& path)
{
    m_content_icon_path = path;
}

void shared_properties_qss_adapter::set_content_icon_style(const QString& style)
{
    m_content_icon_style = style;
}

void shared_properties_qss_adapter::set_settings_icon_path(const QString& path)
{
    m_settings_icon_path = path;
}

void shared_properties_qss_adapter::set_settings_icon_style(const QString& style)
{
    m_settings_icon_style = style;
}
}

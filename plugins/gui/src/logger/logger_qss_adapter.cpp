#include "gui/logger/logger_qss_adapter.h"
#include <QStyle>

namespace hal
{
    LoggerQssAdapter::LoggerQssAdapter(QWidget *parent) : QWidget(parent)
    {
        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
        hide();
    }

    LoggerQssAdapter *LoggerQssAdapter::instance()
    {
        static LoggerQssAdapter* instance = nullptr;
        if(!instance)
            instance = new LoggerQssAdapter();

        return instance;
    }

    QColor LoggerQssAdapter::trace_color() const
    {
        return m_trace_color;
    }

    QColor LoggerQssAdapter::debug_color() const
    {
        return m_debug_color;
    }

    QColor LoggerQssAdapter::info_color() const
    {
        return m_info_color;
    }

    QColor LoggerQssAdapter::warning_color() const
    {
        return m_warning_color;
    }

    QColor LoggerQssAdapter::error_color() const
    {
        return m_error_color;
    }

    QColor LoggerQssAdapter::critical_color() const
    {
        return m_critical_color;
    }

    QColor LoggerQssAdapter::default_color() const
    {
        return m_default_color;
    }

    QColor LoggerQssAdapter::marker_color() const
    {
        return m_marker_color;
    }

    QColor LoggerQssAdapter::trace_highlight() const
    {
        return m_trace_highlight;
    }

    QColor LoggerQssAdapter::debug_highlight() const
    {
        return m_debug_highlight;
    }

    QColor LoggerQssAdapter::info_highlight() const
    {
        return m_info_highlight;
    }

    QColor LoggerQssAdapter::warning_highlight() const
    {
        return m_warning_highlight;
    }

    QColor LoggerQssAdapter::error_highlight() const
    {
        return m_error_highlight;
    }

    QColor LoggerQssAdapter::critical_highlight() const
    {
        return m_critical_highlight;
    }

    QColor LoggerQssAdapter::default_highlight() const
    {
        return m_default_highlight;
    }

    void LoggerQssAdapter::set_trace_color(const QColor &color)
    {
        m_trace_color = color;
    }

    void LoggerQssAdapter::set_debug_color(const QColor &color)
    {
        m_debug_color = color;
    }

    void LoggerQssAdapter::set_info_color(const QColor &color)
    {
        m_info_color = color;
    }

    void LoggerQssAdapter::set_warning_color(const QColor &color)
    {
        m_warning_color = color;
    }

    void LoggerQssAdapter::set_error_color(const QColor &color)
    {
        m_error_color = color;
    }

    void LoggerQssAdapter::set_critical_color(const QColor &color)
    {
        m_critical_color = color;
    }

    void LoggerQssAdapter::set_default_color(const QColor &color)
    {
        m_default_color = color;
    }

    void LoggerQssAdapter::set_marker_color(const QColor &color)
    {
        m_marker_color = color;
    }

    void LoggerQssAdapter::set_trace_highlight(const QColor &color)
    {
        m_trace_highlight = color;
    }

    void LoggerQssAdapter::set_debug_highlight(const QColor &color)
    {
        m_debug_highlight = color;
    }

    void LoggerQssAdapter::set_info_highlight(const QColor &color)
    {
        m_info_highlight = color;
    }

    void LoggerQssAdapter::set_warning_highlight(const QColor &color)
    {
        m_warning_highlight = color;
    }

    void LoggerQssAdapter::set_error_highlight(const QColor &color)
    {
        m_error_highlight = color;
    }

    void LoggerQssAdapter::set_critical_highlight(const QColor &color)
    {
        m_critical_highlight = color;
    }

    void LoggerQssAdapter::set_default_highlight(const QColor &color)
    {
        m_default_highlight = color;
    }
}

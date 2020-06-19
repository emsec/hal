#include "hal_logger/logger_qss_adapter.h"
#include <QStyle>

namespace hal
{
    logger_qss_adapter::logger_qss_adapter(QWidget *parent) : QWidget(parent)
    {
        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
        hide();
    }

    logger_qss_adapter *logger_qss_adapter::instance()
    {
        static logger_qss_adapter* instance = nullptr;
        if(!instance)
            instance = new logger_qss_adapter();

        return instance;
    }

    QColor logger_qss_adapter::trace_color() const
    {
        return m_trace_color;
    }

    QColor logger_qss_adapter::debug_color() const
    {
        return m_debug_color;
    }

    QColor logger_qss_adapter::info_color() const
    {
        return m_info_color;
    }

    QColor logger_qss_adapter::warning_color() const
    {
        return m_warning_color;
    }

    QColor logger_qss_adapter::error_color() const
    {
        return m_error_color;
    }

    QColor logger_qss_adapter::critical_color() const
    {
        return m_critical_color;
    }

    QColor logger_qss_adapter::default_color() const
    {
        return m_default_color;
    }

    QColor logger_qss_adapter::marker_color() const
    {
        return m_marker_color;
    }

    QColor logger_qss_adapter::trace_highlight() const
    {
        return m_trace_highlight;
    }

    QColor logger_qss_adapter::debug_highlight() const
    {
        return m_debug_highlight;
    }

    QColor logger_qss_adapter::info_highlight() const
    {
        return m_info_highlight;
    }

    QColor logger_qss_adapter::warning_highlight() const
    {
        return m_warning_highlight;
    }

    QColor logger_qss_adapter::error_highlight() const
    {
        return m_error_highlight;
    }

    QColor logger_qss_adapter::critical_highlight() const
    {
        return m_critical_highlight;
    }

    QColor logger_qss_adapter::default_highlight() const
    {
        return m_default_highlight;
    }

    void logger_qss_adapter::set_trace_color(const QColor &color)
    {
        m_trace_color = color;
    }

    void logger_qss_adapter::set_debug_color(const QColor &color)
    {
        m_debug_color = color;
    }

    void logger_qss_adapter::set_info_color(const QColor &color)
    {
        m_info_color = color;
    }

    void logger_qss_adapter::set_warning_color(const QColor &color)
    {
        m_warning_color = color;
    }

    void logger_qss_adapter::set_error_color(const QColor &color)
    {
        m_error_color = color;
    }

    void logger_qss_adapter::set_critical_color(const QColor &color)
    {
        m_critical_color = color;
    }

    void logger_qss_adapter::set_default_color(const QColor &color)
    {
        m_default_color = color;
    }

    void logger_qss_adapter::set_marker_color(const QColor &color)
    {
        m_marker_color = color;
    }

    void logger_qss_adapter::set_trace_highlight(const QColor &color)
    {
        m_trace_highlight = color;
    }

    void logger_qss_adapter::set_debug_highlight(const QColor &color)
    {
        m_debug_highlight = color;
    }

    void logger_qss_adapter::set_info_highlight(const QColor &color)
    {
        m_info_highlight = color;
    }

    void logger_qss_adapter::set_warning_highlight(const QColor &color)
    {
        m_warning_highlight = color;
    }

    void logger_qss_adapter::set_error_highlight(const QColor &color)
    {
        m_error_highlight = color;
    }

    void logger_qss_adapter::set_critical_highlight(const QColor &color)
    {
        m_critical_highlight = color;
    }

    void logger_qss_adapter::set_default_highlight(const QColor &color)
    {
        m_default_highlight = color;
    }
}

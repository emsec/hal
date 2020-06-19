#include "python/python_console_qss_adapter.h"
#include <QStyle>

namespace hal
{
    python_console_qss_adapter::python_console_qss_adapter(QWidget *parent) : QWidget(parent)
    {
        style()->unpolish(this);
        style()->polish(this);
        hide();
    }


    python_console_qss_adapter *python_console_qss_adapter::instance()
    {
        static python_console_qss_adapter* instance = nullptr;
        if(!instance)
            instance = new python_console_qss_adapter();

        return instance;
    }

    QColor python_console_qss_adapter::standard_color() const
    {
        return m_standard_color;
    }

    QColor python_console_qss_adapter::error_color() const
    {
        return m_error_color;
    }

    QColor python_console_qss_adapter::promt_color() const
    {
        return m_promt_color;
    }

    void python_console_qss_adapter::set_standard_color(const QColor &color)
    {
        m_standard_color = color;
    }

    void python_console_qss_adapter::set_error_color(const QColor &color)
    {
        m_error_color = color;
    }

    void python_console_qss_adapter::set_promt_color(const QColor &color)
    {
        m_promt_color = color;
    }
}

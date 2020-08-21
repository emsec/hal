#include "gui/python/python_console_qss_adapter.h"
#include <QStyle>

namespace hal
{
    PythonConsoleQssAdapter::PythonConsoleQssAdapter(QWidget *parent) : QWidget(parent)
    {
        style()->unpolish(this);
        style()->polish(this);
        hide();
    }


    PythonConsoleQssAdapter *PythonConsoleQssAdapter::instance()
    {
        static PythonConsoleQssAdapter* instance = nullptr;
        if(!instance)
            instance = new PythonConsoleQssAdapter();

        return instance;
    }

    QColor PythonConsoleQssAdapter::standard_color() const
    {
        return m_standard_color;
    }

    QColor PythonConsoleQssAdapter::error_color() const
    {
        return m_error_color;
    }

    QColor PythonConsoleQssAdapter::promt_color() const
    {
        return m_promt_color;
    }

    void PythonConsoleQssAdapter::set_standard_color(const QColor &color)
    {
        m_standard_color = color;
    }

    void PythonConsoleQssAdapter::set_error_color(const QColor &color)
    {
        m_error_color = color;
    }

    void PythonConsoleQssAdapter::set_promt_color(const QColor &color)
    {
        m_promt_color = color;
    }
}

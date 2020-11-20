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

    QColor PythonConsoleQssAdapter::standardColor() const
    {
        return mStandardColor;
    }

    QColor PythonConsoleQssAdapter::errorColor() const
    {
        return mErrorColor;
    }

    QColor PythonConsoleQssAdapter::promtColor() const
    {
        return mPromtColor;
    }

    void PythonConsoleQssAdapter::setStandardColor(const QColor &color)
    {
        mStandardColor = color;
    }

    void PythonConsoleQssAdapter::setErrorColor(const QColor &color)
    {
        mErrorColor = color;
    }

    void PythonConsoleQssAdapter::setPromtColor(const QColor &color)
    {
        mPromtColor = color;
    }
}

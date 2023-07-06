#include "gui/python/python_console_qss_adapter.h"
#include <QStyle>

namespace hal
{
    PythonConsoleQssAdapter::PythonConsoleQssAdapter(QWidget *parent) : QWidget(parent)
    {
        style()->unpolish(this);
        style()->polish(this);
        mOldStdrd = standardColor().name();
        mOldError = errorColor().name();
        mOldPromt = promtColor().name();
        hide();
    }

    PythonConsoleQssAdapter *PythonConsoleQssAdapter::instance()
    {
        static PythonConsoleQssAdapter* instance = nullptr;
        if(!instance)
            instance = new PythonConsoleQssAdapter();

        return instance;
    }

    QString PythonConsoleQssAdapter::updateStyle(const QString &oldText)
    {
        QString retval;
        style()->unpolish(this);
        style()->polish(this);
        QHash<QString,QString> colorLookup;
        colorLookup[mOldPromt] = promtColor().name();
        colorLookup[mOldError] = errorColor().name();
        colorLookup[mOldStdrd] = standardColor().name();
        int pos0 = 0;
        for (;;)
        {
            int pos1 = oldText.indexOf("color:#",pos0);
            if (pos1 < 0) break;
            retval += oldText.mid(pos0,pos1-pos0+6);
            QString oldColor = oldText.mid(pos1+6,7);
            retval += colorLookup.value(oldColor,standardColor().name());
            pos0 = pos1 + 13;
        }
        retval += oldText.mid(pos0);
        mOldStdrd = standardColor().name();
        mOldError = errorColor().name();
        mOldPromt = promtColor().name();
        return retval;
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

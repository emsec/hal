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

    QColor LoggerQssAdapter::traceColor() const
    {
        return mTraceColor;
    }

    QColor LoggerQssAdapter::debugColor() const
    {
        return mDebugColor;
    }

    QColor LoggerQssAdapter::infoColor() const
    {
        return mInfoColor;
    }

    QColor LoggerQssAdapter::warningColor() const
    {
        return mWarningColor;
    }

    QColor LoggerQssAdapter::errorColor() const
    {
        return mErrorColor;
    }

    QColor LoggerQssAdapter::criticalColor() const
    {
        return mCriticalColor;
    }

    QColor LoggerQssAdapter::defaultColor() const
    {
        return mDefaultColor;
    }

    QColor LoggerQssAdapter::markerColor() const
    {
        return mMarkerColor;
    }

    QColor LoggerQssAdapter::traceHighlight() const
    {
        return mTraceHighlight;
    }

    QColor LoggerQssAdapter::debugHighlight() const
    {
        return mDebugHighlight;
    }

    QColor LoggerQssAdapter::infoHighlight() const
    {
        return mInfoHighlight;
    }

    QColor LoggerQssAdapter::warningHighlight() const
    {
        return mWarningHighlight;
    }

    QColor LoggerQssAdapter::errorHighlight() const
    {
        return mErrorHighlight;
    }

    QColor LoggerQssAdapter::criticalHighlight() const
    {
        return mCriticalHighlight;
    }

    QColor LoggerQssAdapter::defaultHighlight() const
    {
        return mDefaultHighlight;
    }

    void LoggerQssAdapter::setTraceColor(const QColor &color)
    {
        mTraceColor = color;
    }

    void LoggerQssAdapter::setDebugColor(const QColor &color)
    {
        mDebugColor = color;
    }

    void LoggerQssAdapter::setInfoColor(const QColor &color)
    {
        mInfoColor = color;
    }

    void LoggerQssAdapter::setWarningColor(const QColor &color)
    {
        mWarningColor = color;
    }

    void LoggerQssAdapter::setErrorColor(const QColor &color)
    {
        mErrorColor = color;
    }

    void LoggerQssAdapter::setCriticalColor(const QColor &color)
    {
        mCriticalColor = color;
    }

    void LoggerQssAdapter::setDefaultColor(const QColor &color)
    {
        mDefaultColor = color;
    }

    void LoggerQssAdapter::setMarkerColor(const QColor &color)
    {
        mMarkerColor = color;
    }

    void LoggerQssAdapter::setTraceHighlight(const QColor &color)
    {
        mTraceHighlight = color;
    }

    void LoggerQssAdapter::setDebugHighlight(const QColor &color)
    {
        mDebugHighlight = color;
    }

    void LoggerQssAdapter::setInfoHighlight(const QColor &color)
    {
        mInfoHighlight = color;
    }

    void LoggerQssAdapter::setWarningHighlight(const QColor &color)
    {
        mWarningHighlight = color;
    }

    void LoggerQssAdapter::setErrorHighlight(const QColor &color)
    {
        mErrorHighlight = color;
    }

    void LoggerQssAdapter::setCriticalHighlight(const QColor &color)
    {
        mCriticalHighlight = color;
    }

    void LoggerQssAdapter::setDefaultHighlight(const QColor &color)
    {
        mDefaultHighlight = color;
    }
}

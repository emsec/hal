#include "gui/logger/logger_marshall.h"
#include "gui/logger/logger_qss_adapter.h"

#include <QTimer>

#include <algorithm>
#include <functional>
#include <map>

namespace hal
{
    LoggerMarshall::LoggerMarshall(QPlainTextEdit* edit, QObject* parent) : QObject(parent), mEdit(edit)
    {
        mMaxLineCount = 1000;
        mEdit->document()->setMaximumBlockCount(mMaxLineCount);
    }

    void LoggerMarshall::appendLog(spdlog::level::level_enum t, const QString& msg)
    {
        static const QString beginHTML        = "<p><font color=\"";
        static const QString intermediateHTML = "\">";
        static const QString endHTML          = "</font></p>";

        QString html_ready_msg = msg.toHtmlEscaped().replace(" ", "&nbsp;");

        QString color;

        switch (t)
        {
            case spdlog::level::level_enum::trace:
                color = LoggerQssAdapter::instance()->traceColor().name();
                break;
            case spdlog::level::level_enum::debug:
                color = LoggerQssAdapter::instance()->debugColor().name();
                break;
            case spdlog::level::level_enum::info:
                color = LoggerQssAdapter::instance()->infoColor().name();
                break;
            case spdlog::level::level_enum::warn:
                color = LoggerQssAdapter::instance()->warningColor().name();
                break;
            case spdlog::level::level_enum::err:
                color = LoggerQssAdapter::instance()->errorColor().name();
                break;
            case spdlog::level::level_enum::critical:
                color = LoggerQssAdapter::instance()->criticalColor().name();
                break;
            default:
                color = LoggerQssAdapter::instance()->defaultColor().name();
                break;
        }

        mEdit->appendHtml(beginHTML + color + intermediateHTML + html_ready_msg + endHTML);
    }
}

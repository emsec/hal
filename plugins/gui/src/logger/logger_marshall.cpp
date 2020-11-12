#include "gui/logger/logger_marshall.h"
#include "gui/logger/filter_item.h"
#include <QTimer>
#include <algorithm>
#include <functional>
#include <map>
#include "gui/logger/logger_qss_adapter.h"

namespace hal
{
    LoggerMarshall::LoggerMarshall(QPlainTextEdit* edit, QObject* parent) : QObject(parent), mEdit(edit)
    {
        mMaxLineCount = 1000;
        mEdit->document()->setMaximumBlockCount(mMaxLineCount);
        connect(mEdit, &QPlainTextEdit::cursorPositionChanged, this, &LoggerMarshall::highlightCurrentLine);
    }

    void LoggerMarshall::appendLog(spdlog::level::level_enum t, const QString& msg, FilterItem* filter)
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
        QString msg_to_print;
        if (!filter)
            msg_to_print = beginHTML + color + intermediateHTML + html_ready_msg + endHTML;
        else
        {
            FilterItem::Rule rule;
            switch (t)
            {
                case spdlog::level::level_enum::trace:
                    rule = filter->mTrace;
                    break;
                case spdlog::level::level_enum::debug:
                    rule = filter->mDebug;
                    break;
                case spdlog::level::level_enum::info:
                    rule = filter->mInfo;
                    break;
                case spdlog::level::level_enum::warn:
                    rule = filter->mWarning;
                    break;
                case spdlog::level::level_enum::err:
                    rule = filter->mError;
                    break;
                case spdlog::level::level_enum::critical:
                    rule = filter->mCritical;
                    break;
                default:
                    rule = filter->mDefault;
                    break;
            }

            if (rule != FilterItem::Rule::HideAll)
            {
                bool match = false;
                for (QString key : filter->mKeywords)
                {
                    if (msg.contains(key, Qt::CaseInsensitive))
                    {
                        match = true;
                        break;
                    }
                }

                if (match)
                {
                    switch (t)
                    {
                        case spdlog::level::level_enum::trace:
                            color = LoggerQssAdapter::instance()->traceHighlight().name();
                            break;
                        case spdlog::level::level_enum::debug:
                            color = LoggerQssAdapter::instance()->debugHighlight().name();
                            break;
                        case spdlog::level::level_enum::info:
                            color = LoggerQssAdapter::instance()->infoHighlight().name();
                            break;
                        case spdlog::level::level_enum::warn:
                            color = LoggerQssAdapter::instance()->warningHighlight().name();
                            break;
                        case spdlog::level::level_enum::err:
                            color = LoggerQssAdapter::instance()->errorHighlight().name();
                            break;
                        case spdlog::level::level_enum::critical:
                            color = LoggerQssAdapter::instance()->criticalHighlight().name();
                            break;
                        default:
                            color = LoggerQssAdapter::instance()->defaultHighlight().name();
                            break;
                    }
                    msg_to_print = beginHTML + color + intermediateHTML + html_ready_msg + endHTML;
                }
                else
                {
                    if (rule == FilterItem::Rule::ShowAll)
                        msg_to_print = beginHTML + color + intermediateHTML + html_ready_msg + endHTML;
                    else
                        return;
                }
            }
            else
                return;
        }
        mEdit->appendHtml(msg_to_print);
    }

    void LoggerMarshall::highlightCurrentLine()
    {
    //    QList<QTextEdit::ExtraSelection> extraSelections;
    //    QTextEdit::ExtraSelection selection;
    //    QColor lineColor = QColor(48, 49, 49, 255);

    //    selection.format.setBackground(lineColor);
    //    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    //    selection.cursor = mEdit->textCursor();
    //    selection.cursor.clearSelection();
    //    extraSelections.append(selection);

    //    mEdit->setExtraSelections(extraSelections);
    }
}

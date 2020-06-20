#include "hal_logger/hal_logger_marshall.h"
#include "hal_logger/hal_filter_item.h"
#include <QTimer>
#include <algorithm>
#include <functional>
#include <map>
#include "hal_logger/logger_qss_adapter.h"

namespace hal
{
    HalLoggerMarshall::HalLoggerMarshall(QPlainTextEdit* edit, QObject* parent) : QObject(parent), m_edit(edit)
    {
        m_max_line_count = 1000;
        m_edit->document()->setMaximumBlockCount(m_max_line_count);
        connect(m_edit, &QPlainTextEdit::cursorPositionChanged, this, &HalLoggerMarshall::highlight_current_line);
    }

    HalLoggerMarshall::~HalLoggerMarshall()
    {
        LogManager::get_instance().get_gui_callback().remove_callback("gui");
    }

    void HalLoggerMarshall::append_log(spdlog::level::level_enum t, const QString& msg, HalFilterItem* filter)
    {
        static const QString beginHTML        = "<p><font color=\"";
        static const QString intermediateHTML = "\">";
        static const QString endHTML          = "</font></p>";

        QString html_ready_msg = msg.toHtmlEscaped().replace(" ", "&nbsp;");

        QString color;

        switch (t)
        {
            case spdlog::level::level_enum::trace:
                color = LoggerQssAdapter::instance()->trace_color().name();
                break;
            case spdlog::level::level_enum::debug:
                color = LoggerQssAdapter::instance()->debug_color().name();
                break;
            case spdlog::level::level_enum::info:
                color = LoggerQssAdapter::instance()->info_color().name();
                break;
            case spdlog::level::level_enum::warn:
                color = LoggerQssAdapter::instance()->warning_color().name();
                break;
            case spdlog::level::level_enum::err:
                color = LoggerQssAdapter::instance()->error_color().name();
                break;
            case spdlog::level::level_enum::critical:
                color = LoggerQssAdapter::instance()->critical_color().name();
                break;
            default:
                color = LoggerQssAdapter::instance()->default_color().name();
                break;
        }
        QString msg_to_print;
        if (!filter)
            msg_to_print = beginHTML + color + intermediateHTML + html_ready_msg + endHTML;
        else
        {
            HalFilterItem::rule rule;
            switch (t)
            {
                case spdlog::level::level_enum::trace:
                    rule = filter->m_trace;
                    break;
                case spdlog::level::level_enum::debug:
                    rule = filter->m_debug;
                    break;
                case spdlog::level::level_enum::info:
                    rule = filter->m_info;
                    break;
                case spdlog::level::level_enum::warn:
                    rule = filter->m_warning;
                    break;
                case spdlog::level::level_enum::err:
                    rule = filter->m_error;
                    break;
                case spdlog::level::level_enum::critical:
                    rule = filter->m_critical;
                    break;
                default:
                    rule = filter->m_default;
                    break;
            }

            if (rule != HalFilterItem::rule::HideAll)
            {
                bool match = false;
                for (QString key : filter->m_keywords)
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
                            color = LoggerQssAdapter::instance()->trace_highlight().name();
                            break;
                        case spdlog::level::level_enum::debug:
                            color = LoggerQssAdapter::instance()->debug_highlight().name();
                            break;
                        case spdlog::level::level_enum::info:
                            color = LoggerQssAdapter::instance()->info_highlight().name();
                            break;
                        case spdlog::level::level_enum::warn:
                            color = LoggerQssAdapter::instance()->warning_highlight().name();
                            break;
                        case spdlog::level::level_enum::err:
                            color = LoggerQssAdapter::instance()->error_highlight().name();
                            break;
                        case spdlog::level::level_enum::critical:
                            color = LoggerQssAdapter::instance()->critical_highlight().name();
                            break;
                        default:
                            color = LoggerQssAdapter::instance()->default_highlight().name();
                            break;
                    }
                    msg_to_print = beginHTML + color + intermediateHTML + html_ready_msg + endHTML;
                }
                else
                {
                    if (rule == HalFilterItem::rule::ShowAll)
                        msg_to_print = beginHTML + color + intermediateHTML + html_ready_msg + endHTML;
                    else
                        return;
                }
            }
            else
                return;
        }
        m_edit->appendHtml(msg_to_print);
    }

    void HalLoggerMarshall::highlight_current_line()
    {
    //    QList<QTextEdit::ExtraSelection> extraSelections;
    //    QTextEdit::ExtraSelection selection;
    //    QColor lineColor = QColor(48, 49, 49, 255);

    //    selection.format.setBackground(lineColor);
    //    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    //    selection.cursor = m_edit->textCursor();
    //    selection.cursor.clearSelection();
    //    extraSelections.append(selection);

    //    m_edit->setExtraSelections(extraSelections);
    }
}

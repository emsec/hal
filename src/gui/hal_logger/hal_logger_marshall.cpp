#include "hal_logger/hal_logger_marshall.h"
#include "hal_logger/hal_filter_item.h"
#include <QTimer>
#include <algorithm>
#include <functional>
#include <map>

hal_logger_marshall::hal_logger_marshall(QPlainTextEdit* edit, QObject* parent) : QObject(parent), m_edit(edit)
{
    m_max_line_count = 1000;
    m_edit->document()->setMaximumBlockCount(m_max_line_count);
    connect(m_edit, &QPlainTextEdit::cursorPositionChanged, this, &hal_logger_marshall::highlight_current_line);
}

hal_logger_marshall::~hal_logger_marshall()
{
    log_manager::get_instance().get_gui_callback().remove_callback("gui");
}

void hal_logger_marshall::append_log(spdlog::level::level_enum t, const QString& msg, hal_filter_item* filter)
{
    static const QString beginHTML        = "<p><font color=\"";
    static const QString intermediateHTML = "\">";
    static const QString endHTML          = "</font></p>";

    static const QString traceColor    = "#90C023";
    static const QString debugColor    = "#90C023";
    //static const QString infoColor     = "#287BDE";
    static const QString infoColor     = "#e4e4e4";
    static const QString warningColor  = "#CC7832";
    static const QString errorColor    = "#FF0000";
    static const QString criticalColor = "#FF6B68";
    static const QString defaultColor  = "#BBBBBB";

    static const QString traceHighlight    = "#FFEF0B";
    static const QString debugHighlight    = "#FFEF0B";
    static const QString infoHighlight     = "#00B7E0";
    static const QString warningHighlight  = "#FFEF0B";
    static const QString errorHighlight    = "#FFEF0B";
    static const QString criticalHighlight = "#FFEF0B";
    static const QString defaultHighlight  = "#FFEF0B";

    static const QString markerColor = "#FFEF0B";

    QString color;

    switch (t)
    {
        case spdlog::level::level_enum::trace:
            color = traceColor;
            break;
        case spdlog::level::level_enum::debug:
            color = debugColor;
            break;
        case spdlog::level::level_enum::info:
            color = infoColor;
            break;
        case spdlog::level::level_enum::warn:
            color = warningColor;
            break;
        case spdlog::level::level_enum::err:
            color = errorColor;
            break;
        case spdlog::level::level_enum::critical:
            color = criticalColor;
            break;
        default:
            color = defaultColor;
            break;
    }
    QString msg_to_print;
    if (!filter)
        msg_to_print = beginHTML + color + intermediateHTML + msg + endHTML;
    else
    {
        hal_filter_item::rule rule;
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

        if (rule != hal_filter_item::rule::HideAll)
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
                        color = traceHighlight;
                        break;
                    case spdlog::level::level_enum::debug:
                        color = debugHighlight;
                        break;
                    case spdlog::level::level_enum::info:
                        color = infoHighlight;
                        break;
                    case spdlog::level::level_enum::warn:
                        color = warningHighlight;
                        break;
                    case spdlog::level::level_enum::err:
                        color = errorHighlight;
                        break;
                    case spdlog::level::level_enum::critical:
                        color = criticalHighlight;
                        break;
                    default:
                        color = defaultHighlight;
                        break;
                }
                msg_to_print = beginHTML + color + intermediateHTML + msg + endHTML;
            }
            else
            {
                if (rule == hal_filter_item::rule::ShowAll)
                    msg_to_print = beginHTML + color + intermediateHTML + msg + endHTML;
                else
                    return;
            }
        }
        else
            return;
    }
    m_edit->appendHtml(msg_to_print);
}

void hal_logger_marshall::highlight_current_line()
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

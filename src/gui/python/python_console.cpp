#include "python/python_console.h"

#include "core/log.h"
#include "gui_globals.h"

#include <QKeyEvent>

#include <QDebug>

#include "python/python_console_history.h"

#include "python/python_console_qss_adapter.h"

namespace hal
{
    python_console::python_console(QWidget* parent)
        : QTextEdit(parent), m_standard_prompt(">>> "), m_compound_prompt("... "), m_prompt_block_number(0), m_prompt_length(0), m_prompt_end_position(0), m_compound_prompt_end_position(0),
          m_in_compound_prompt(false), m_in_completion(false), m_current_compound_input(""), m_current_input(""), m_current_history_index(-1), m_current_completer_index(0),
          m_history(std::make_shared<python_console_history>())
    {
        this->document()->setMaximumBlockCount(1000);
        setFrameStyle(QFrame::NoFrame);
        setUndoRedoEnabled(false);
        ensureCursorVisible();

        m_standard_color = python_console_qss_adapter::instance()->standard_color();
        m_error_color = python_console_qss_adapter::instance()->error_color();
        m_prompt_color = python_console_qss_adapter::instance()->promt_color();
        g_python_context->set_console(this);
        g_python_context->interpret("print(\"Python \" + sys.version)", false);
        g_python_context->interpret("print(sys.executable + \" on \" + sys.platform)", false);
        display_prompt();
    }

    void python_console::keyPressEvent(QKeyEvent* e)
    {
        auto cursor = textCursor();
        if (textCursor().hasSelection())
        {
            if (e->matches(QKeySequence::Copy))
            {
                e->accept();
                copy();
                return;
            }
            else if (e->matches(QKeySequence::Cut))
            {
                // cut cutable stuff and copy rest
                return;
            }
            else if (e->key() == Qt::Key_Control)
            {
                return;
            }
        }

        switch (e->key())
        {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                interpret_command();
                m_current_history_index = -1;
                return;

            case Qt::Key_Backspace:
                if (textCursor().hasSelection())
                {
                    if (textCursor().selectionStart() >= m_prompt_end_position)
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    if (textCursor().selectionStart() > m_prompt_end_position)
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }

            case Qt::Key_Up:
                handle_up_key_pressed();
                return;

            case Qt::Key_Down:
                handle_down_key_pressed();
                return;

            case Qt::Key_Left:
                if (e->modifiers() & Qt::ControlModifier)    // Needed for macOS
                {
                    cursor.setPosition(m_prompt_end_position);
                    setTextCursor(cursor);
                    return;
                }
                if (textCursor().hasSelection())
                {
                    if (textCursor().selectionStart() >= m_prompt_end_position)
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    if (textCursor().selectionStart() > m_prompt_end_position)
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }
            case Qt::Key_Home:
                cursor.setPosition(m_prompt_end_position);
                setTextCursor(cursor);
                return;
            case Qt::Key_End:
                cursor.movePosition(QTextCursor::End);
                setTextCursor(cursor);
                return;
            case Qt::Key_Right:
                if (e->modifiers() & Qt::ControlModifier)    // Needed for macOS
                {
                    cursor.movePosition(QTextCursor::End);
                    setTextCursor(cursor);
                    return;
                }
                break;

            case Qt::Key_Tab:
                handle_tab_key_pressed();
                return;

            default:
                if (textCursor().selectionStart() < m_prompt_end_position)
                {
                    moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                }
                m_in_completion         = false;
                m_current_history_index = -1;
        }
        QTextEdit::keyPressEvent(e);
    }

    void python_console::mousePressEvent(QMouseEvent* event)
    {
        //        m_position = textCursor().position();
        //        if (event->button() == Qt::MidButton)
        //        {
        //            copy();
        //            QTextCursor cursor = cursorForPosition(event->pos());
        //            setTextCursor(cursor);
        //            paste();
        //            return;
        //        }
        //        cursor.movePosition(QTextCursor::End);
        QTextEdit::mousePressEvent(event);
    }

    void python_console::insertAtEnd(const QString& text, QColor textColor)
    {
        moveCursor(QTextCursor::End);
        setTextColor(textColor);
        insertPlainText(QString(text));
    }

    void python_console::handle_stdout(const QString& output)
    {
        insertAtEnd(output, m_standard_color);
    }

    void python_console::handle_error(const QString& output)
    {
        QString append_out = output;
        if (!append_out.endsWith("\n"))
            append_out += "\n";
        insertAtEnd(append_out, m_error_color);
    }

    void python_console::clear()
    {
        QTextEdit::clear();
    }

    void python_console::display_prompt()
    {
        //QTextCursor cursor = textCursor();
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::End);
        //DEBUG SAVE FORMATS AS MEMBERS
        QTextCharFormat format;
        format.setForeground(m_prompt_color);
        cursor.setCharFormat(format);
        if (m_in_compound_prompt)
        {
            cursor.insertText(m_compound_prompt);
            if (m_compound_prompt_end_position < 0)
            {
                m_compound_prompt_end_position = m_prompt_end_position;
            }
        }
        else
        {
            cursor.insertText(m_standard_prompt);
            m_compound_prompt_end_position = -1;
        }
        cursor.movePosition(QTextCursor::EndOfLine);
        setTextCursor(cursor);

        //    m_prompt_block_number = textCursor().blockNumber();
        m_prompt_length       = m_standard_prompt.length();
        m_prompt_end_position = textCursor().position();
    }

    void python_console::interpret_command()
    {
        QString input      = get_current_command();
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n");
        if (!input.isEmpty())
        {
            //        g_python_context->add_history(input);
            m_history->add_history(input.toStdString());
        }
        if ((!m_in_compound_prompt && g_python_context->check_complete_statement(input) != 0) || (m_in_compound_prompt && input.isEmpty() && g_python_context->check_complete_statement(input) != 0))
        {
            m_current_compound_input += input;
            if (m_in_compound_prompt)
            {
                g_python_context->interpret(m_current_compound_input, true);
            }
            else
            {
                g_python_context->interpret(input, false);
            }
            m_in_compound_prompt     = false;
            m_current_compound_input = "";
            display_prompt();
        }
        else
        {
            m_current_compound_input += input + "\n";
            m_in_compound_prompt = true;
            display_prompt();
        }
        m_history->update_from_file();
        //qDebug() << m_current_compound_input;
    }

    QString python_console::get_current_command()
    {
        QTextCursor cursor = textCursor();

        cursor.setPosition(m_prompt_end_position);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

        QString command = cursor.selectedText();
        cursor.clearSelection();
        return command;
    }

    void python_console::replace_current_command(const QString& new_command)
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(m_prompt_end_position);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.insertText(new_command);
    }

    void python_console::append_to_current_command(const QString& appendix)
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(appendix);
    }

    bool python_console::selection_editable()
    {
        QTextCursor cursor(document());
        cursor.setPosition(textCursor().selectionStart());
        return false;
    }

    void python_console::handle_up_key_pressed()
    {
        auto length = m_history->size() - 1;

        if (m_current_history_index == -1)
        {
            m_current_input         = get_current_command();
            m_current_history_index = length;
            replace_current_command(QString::fromStdString(m_history->get_history_item(m_current_history_index)));
        }
        else
        {
            if (m_current_history_index == 0)
            {
                return;
            }

            m_current_history_index--;
            replace_current_command(QString::fromStdString(m_history->get_history_item(m_current_history_index)));
        }
    }

    void python_console::handle_down_key_pressed()
    {
        auto length = m_history->size() - 1;

        if (m_current_history_index == -1)
        {
            return;
        }

        if (m_current_history_index == length)
        {
            m_current_history_index = -1;
            replace_current_command(m_current_input);
        }
        else
        {
            m_current_history_index++;
            replace_current_command(QString::fromStdString(m_history->get_history_item(m_current_history_index)));
        }
    }

    void python_console::handle_tab_key_pressed()
    {
        if (m_in_compound_prompt)
        {
            m_current_input = m_current_compound_input + get_current_command();
        }
        else
        {
            m_current_input = get_current_command();
        }
        QString current_line = get_current_command();
        if (current_line.isEmpty())
        {
            insertPlainText("\t");
        }
        else
        {
            log_info("python", "completing: '{}'", m_current_input.toStdString());
            auto r = g_python_context->complete(m_current_input, true);
            if (r.size() == 1)
            {
                append_to_current_command(QString::fromStdString(std::get<1>(r.at(0))));
            }
            else if (r.size() > 1)
            {
                m_in_completion         = true;
                QString candidates      = "\n";
                QString matching_prefix = QString::fromStdString(std::get<1>(r[0]));
                for (auto& tup : r)
                {
                    auto candidate  = QString::fromStdString(std::get<0>(tup));
                    auto completion = QString::fromStdString(std::get<1>(tup));
                    candidates += candidate + "    ";
                    for (int i = 0; i < matching_prefix.size() && i < completion.size(); ++i)
                    {
                        if (matching_prefix[i] != completion[i])
                        {
                            matching_prefix = matching_prefix.mid(0, i);
                        }
                    }
                }
                candidates += "\n";
                handle_stdout(candidates);
                display_prompt();
                replace_current_command(current_line + matching_prefix);
            }
        }
    }
}

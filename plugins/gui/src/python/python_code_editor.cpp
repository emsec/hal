#include "python/python_code_editor.h"
#include "python/python_editor_code_completion_dialog.h"

#include "code_editor/syntax_highlighter/python_syntax_highlighter.h"
#include "core/log.h"
#include "gui_globals.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QTextDocumentFragment>
#include <QVBoxLayout>
#include <QShortcut>

namespace hal
{
    PythonCodeEditor::PythonCodeEditor(QWidget *parent) : CodeEditor(parent), m_uuid(QUuid::createUuid())
    {
        QShortcut* redo_shortcut = new QShortcut(QKeySequence(tr("Ctrl+y")), this);
        connect(redo_shortcut, &QShortcut::activated, this, &PythonCodeEditor::handle_redo_requested);

        m_base_file_modified = false;
    }

    void PythonCodeEditor::keyPressEvent(QKeyEvent* e)
    {
        Q_EMIT key_pressed(e);
        if (textCursor().hasSelection() && !(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab))
        {
            QPlainTextEdit::keyPressEvent(e);
        }
        else
        {
            switch(e->key())
            {
                case Qt::Key_Tab:       handle_tab_key_pressed();               break;
                case Qt::Key_Backtab:   handle_shift_tab_key_pressed();         break;
                case Qt::Key_Return:    handle_return_key_pressed();            break;
                case Qt::Key_Backspace: handle_backspace_key_pressed(e);        break;
                case Qt::Key_Delete:    handle_delete_key_pressed(e);           break;
                case Qt::Key_Insert:    handle_insert_key_pressed();            break;
                default:                QPlainTextEdit::keyPressEvent(e);
            }
        }
    }

    void PythonCodeEditor::handle_shift_tab_key_pressed()
    {
        PythonCodeEditor::indent_selection(false);
    }

    void PythonCodeEditor::handle_tab_key_pressed()
    {
        PythonCodeEditor::indent_selection(true);
    }

    void PythonCodeEditor::handle_return_key_pressed()
    {
        QString current_line;
        {
            auto cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            current_line = cursor.selectedText();
        }
        u32 num_spaces = 0;
        for (const auto& c : current_line)
        {
            if (c != ' ')
            {
                break;
            }
            num_spaces++;
        }

        auto trimmed = current_line.trimmed();
        if (!trimmed.isEmpty() && trimmed.at(trimmed.size() - 1) == ':')
        {
            num_spaces += 4;
        }

        num_spaces -= num_spaces % 4;
        insertPlainText("\n");
        for (u32 i = 0; i < num_spaces / 4; ++i)
        {
            insertPlainText("    ");
        }
        ensureCursorVisible();
    }

    void PythonCodeEditor::handle_backspace_key_pressed(QKeyEvent* e)
    {
        QString current_line;
        {
            auto cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            current_line = cursor.selectedText();
        }

        if (!current_line.isEmpty() && current_line.trimmed().isEmpty() && current_line.size() % 4 == 0)
        {
            auto cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
            cursor.removeSelectedText();
            return;
        }

        QPlainTextEdit::keyPressEvent(e);
    }

    void PythonCodeEditor::handle_delete_key_pressed(QKeyEvent* e)
    {
        auto cursor = textCursor();
        if (cursor.positionInBlock() % 4 == 0)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
            if (cursor.selectedText() == "    ")
            {
                cursor.removeSelectedText();
                return;
            }
        }

        QPlainTextEdit::keyPressEvent(e);
    }

    void PythonCodeEditor::handle_insert_key_pressed()
    {
        setOverwriteMode(!overwriteMode());
    }

    void PythonCodeEditor::handle_redo_requested()
    {
        redo();
    }

    void PythonCodeEditor::handle_autocomplete()
    {
        auto cursor = textCursor();
            // auto row    = cursor.blockNumber();
            // auto col    = cursor.positionInBlock();
            cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            auto text       = cursor.selectedText().replace(QChar(0x2029), '\n');
            auto candidates = g_python_context->complete(text, false);

            if (candidates.size() == 1)
            {
                textCursor().insertText(QString::fromStdString(std::get<1>(candidates.at(0))));
            }
            else if (candidates.size() > 1)
            {
                auto dialog = new PythonEditorCodeCompletionDialog(this, candidates);
                connect(dialog, &PythonEditorCodeCompletionDialog::completionSelected, this, &PythonCodeEditor::perform_code_completion);
                auto menu_width  = dialog->width();
                auto menu_height = dialog->height();

                auto desk_rect   = QApplication::desktop()->screenGeometry();
                auto desk_width  = desk_rect.width();
                auto desk_height = desk_rect.height();
                auto anchor      = this->cursorRect().bottomRight();
                anchor.setX(anchor.x() + viewportMargins().left());
                auto anchor_global = this->mapToGlobal(anchor);

                if (anchor_global.x() + menu_width > desk_width)
                {
                    anchor.setX(anchor.x() - menu_width);
                    anchor_global = this->mapToGlobal(anchor);
                }

                if (anchor_global.y() + menu_height > desk_height)
                {
                    anchor.setY(cursorRect().topRight().y() - menu_height);
                    anchor_global = this->mapToGlobal(anchor);
                }
                dialog->move(anchor_global);
                dialog->exec();
            }
    }

    int PythonCodeEditor::next_indent(bool indentUnindent, int current_indent)
    {
        int next_indent;
        if (indentUnindent)
        {
            next_indent = 4 - (current_indent % 4);
        }
        else
        {
            next_indent = current_indent % 4;
            if (next_indent == 0)
            {
                next_indent = 4;
            }
        }
        return next_indent;
    }

    void PythonCodeEditor::indent_selection(bool indent)
    {
        auto cursor = textCursor();
        bool preSelected = cursor.hasSelection();
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        if (preSelected)
        {
            // expand selection to the start of the first line
            cursor.setPosition(end, QTextCursor::MoveAnchor);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        }
        else
        {
            // select first line (to decide auto-completion)
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            QString line_start_to_cursor = cursor.selection().toPlainText();
            bool onlySpaces = true;
            for (const auto& c : line_start_to_cursor)
            {
                if (c != ' ')
                {
                    onlySpaces = false;
                    break;
                }
            }
            if (indent && !onlySpaces)
            {
                // if the cursor is in a word without a selection, show autocompletion menu
                // (skip this if we are un-indenting, meaning Shift+Tab has been pressed)
                handle_autocomplete();
                return;
            }
            // select whole line (to count total spaces)
            // don't mess with this order, it'll affect the text insertion later on
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        }

        QStringList selected_lines = cursor.selection().toPlainText().split('\n');
        cursor.clearSelection();

        // calculate number of spaces at the beginning of the first line and
        // check if the line consists entirely of spaces or is empty
        int n_spaces = 0;
        if (!selected_lines.isEmpty())
        {
            for (const auto& c : selected_lines[0])
            {
                if (c != ' ')
                {
                    break;
                }
                n_spaces++;
            }
        }
        // calculate indent to use for all selected lines based on the amount of
        // spaces needed to align the first line to the next_indent 4-block
        const int constant_indent = next_indent(indent, n_spaces);

        // (un)indent all selected lines
        const int size = selected_lines.size();
        QString padding;
        if (indent)
        {
            padding = "";
            padding.fill(' ', constant_indent);
        }
        cursor.beginEditBlock();
        for (int i = 0; i < size; i++)
        {
            if (indent)
            {
                cursor.insertText(padding);
                end+=constant_indent;
            }
            else
            {
                QString current_line = selected_lines[i];
                // count how many spaces there really are so we don't cut off text
                int spaces = 0;
                while(spaces < constant_indent)
                {
                    if (current_line[spaces] != ' ')
                    {
                        break;
                    }
                    spaces++;
                }
                // un-indent line by removing spaces
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, spaces);
                cursor.removeSelectedText();
                end-=spaces;
            }
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        }
        cursor.endEditBlock();
        // restore selection
        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
    }

    void PythonCodeEditor::perform_code_completion(std::tuple<std::string, std::string> completion)
    {
        textCursor().insertText(QString::fromStdString(std::get<1>(completion)));
    }

    QString PythonCodeEditor::get_file_name()
    {
        return m_file_name;
    }

    void PythonCodeEditor::set_file_name(const QString name)
    {
        m_file_name = name;
    }

    void PythonCodeEditor::set_base_file_modified(bool base_file_modified)
    {
        m_base_file_modified = base_file_modified;
    }

    bool PythonCodeEditor::is_base_file_modified()
    {
        return m_base_file_modified;

    }

    QUuid PythonCodeEditor::get_uuid() const
    {
        return m_uuid;
    }
}

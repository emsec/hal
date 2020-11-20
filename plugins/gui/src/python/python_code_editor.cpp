#include "gui/python/python_code_editor.h"
#include "gui/python/python_editor_code_completion_dialog.h"

#include "gui/code_editor/syntax_highlighter/python_syntax_highlighter.h"
#include "hal_core/utilities/log.h"
#include "gui/gui_globals.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QTextDocumentFragment>
#include <QVBoxLayout>
#include <QShortcut>

namespace hal
{
    PythonCodeEditor::PythonCodeEditor(QWidget *parent) : CodeEditor(parent), mUuid(QUuid::createUuid())
    {
        QShortcut* redo_shortcut = new QShortcut(QKeySequence(tr("Ctrl+y")), this);
        connect(redo_shortcut, &QShortcut::activated, this, &PythonCodeEditor::handleRedoRequested);

        mBaseFileModified = false;
    }

    void PythonCodeEditor::keyPressEvent(QKeyEvent* e)
    {
        Q_EMIT keyPressed(e);
        if (textCursor().hasSelection() && !(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab))
        {
            QPlainTextEdit::keyPressEvent(e);
        }
        else
        {
            switch(e->key())
            {
                case Qt::Key_Tab:       handleTabKeyPressed();               break;
                case Qt::Key_Backtab:   handleShiftTabKeyPressed();         break;
                case Qt::Key_Return:    handleReturnKeyPressed();            break;
                case Qt::Key_Backspace: handleBackspaceKeyPressed(e);        break;
                case Qt::Key_Delete:    handleDeleteKeyPressed(e);           break;
                case Qt::Key_Insert:    handleInsertKeyPressed();            break;
                default:                QPlainTextEdit::keyPressEvent(e);
            }
        }
    }

    void PythonCodeEditor::handleShiftTabKeyPressed()
    {
        PythonCodeEditor::indentSelection(false);
    }

    void PythonCodeEditor::handleTabKeyPressed()
    {
        PythonCodeEditor::indentSelection(true);
    }

    void PythonCodeEditor::handleReturnKeyPressed()
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

    void PythonCodeEditor::handleBackspaceKeyPressed(QKeyEvent* e)
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

    void PythonCodeEditor::handleDeleteKeyPressed(QKeyEvent* e)
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

    void PythonCodeEditor::handleInsertKeyPressed()
    {
        setOverwriteMode(!overwriteMode());
    }

    void PythonCodeEditor::handleRedoRequested()
    {
        redo();
    }

    void PythonCodeEditor::handleAutocomplete()
    {
        auto cursor = textCursor();
            // auto row    = cursor.blockNumber();
            // auto col    = cursor.positionInBlock();
            cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            auto text       = cursor.selectedText().replace(QChar(0x2029), '\n');
            auto candidates = gPythonContext->complete(text, false);

            if (candidates.size() == 1)
            {
                textCursor().insertText(QString::fromStdString(std::get<1>(candidates.at(0))));
            }
            else if (candidates.size() > 1)
            {
                auto dialog = new PythonEditorCodeCompletionDialog(this, candidates);
                connect(dialog, &PythonEditorCodeCompletionDialog::completionSelected, this, &PythonCodeEditor::performCodeCompletion);
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

    int PythonCodeEditor::nextIndent(bool indentUnindent, int current_indent)
    {
        int nextIndent;
        if (indentUnindent)
        {
            nextIndent = 4 - (current_indent % 4);
        }
        else
        {
            nextIndent = current_indent % 4;
            if (nextIndent == 0)
            {
                nextIndent = 4;
            }
        }
        return nextIndent;
    }

    void PythonCodeEditor::indentSelection(bool indent)
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
                handleAutocomplete();
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
        // spaces needed to align the first line to the nextIndent 4-block
        const int constant_indent = nextIndent(indent, n_spaces);

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

    void PythonCodeEditor::performCodeCompletion(std::tuple<std::string, std::string> completion)
    {
        textCursor().insertText(QString::fromStdString(std::get<1>(completion)));
    }

    QString PythonCodeEditor::getFileName()
    {
        return mFileName;
    }

    void PythonCodeEditor::set_file_name(const QString name)
    {
        mFileName = name;
    }

    void PythonCodeEditor::setBaseFileModified(bool base_file_modified)
    {
        mBaseFileModified = base_file_modified;
    }

    bool PythonCodeEditor::isBaseFileModified()
    {
        return mBaseFileModified;

    }

    QUuid PythonCodeEditor::getUuid() const
    {
        return mUuid;
    }
}

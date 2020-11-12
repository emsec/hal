#include "gui/python/python_console.h"

#include "hal_core/utilities/log.h"
#include "gui/gui_globals.h"

#include <QKeyEvent>

#include <QDebug>

#include "gui/python/python_console_history.h"

#include "gui/python/python_console_qss_adapter.h"

namespace hal
{
    PythonConsole::PythonConsole(QWidget* parent)
        : QTextEdit(parent), mStandardPrompt(">>> "), mCompoundPrompt("... "), mPromptBlockNumber(0), mPromptLength(0), mPromptEndPosition(0), mCompoundPromptEndPosition(0),
          mInCompoundPrompt(false), mInCompletion(false), mCurrentCompoundInput(""), mCurrentInput(""), mCurrentHistoryIndex(-1), mCurrentCompleterIndex(0),
          mHistory(std::make_shared<PythonConsoleHistory>())
    {
        this->document()->setMaximumBlockCount(1000);
        setFrameStyle(QFrame::NoFrame);
        setUndoRedoEnabled(false);
        ensureCursorVisible();

        mStandardColor = PythonConsoleQssAdapter::instance()->standardColor();
        mErrorColor = PythonConsoleQssAdapter::instance()->errorColor();
        mPromptColor = PythonConsoleQssAdapter::instance()->promtColor();
        gPythonContext->setConsole(this);
        gPythonContext->interpret("print(\"Python \" + sys.version)", false);
        gPythonContext->interpret("print(sys.executable + \" on \" + sys.platform)", false);
        displayPrompt();
    }

    void PythonConsole::keyPressEvent(QKeyEvent* e)
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
                interpretCommand();
                mCurrentHistoryIndex = -1;
                return;

            case Qt::Key_Backspace:
                if (textCursor().hasSelection())
                {
                    if (textCursor().selectionStart() >= mPromptEndPosition)
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
                    if (textCursor().selectionStart() > mPromptEndPosition)
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }

            case Qt::Key_Up:
                handleUpKeyPressed();
                return;

            case Qt::Key_Down:
                handleDownKeyPressed();
                return;

            case Qt::Key_Left:
                if (e->modifiers() & Qt::ControlModifier)    // Needed for macOS
                {
                    cursor.setPosition(mPromptEndPosition);
                    setTextCursor(cursor);
                    return;
                }
                if (textCursor().hasSelection())
                {
                    if (textCursor().selectionStart() >= mPromptEndPosition)
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
                    if (textCursor().selectionStart() > mPromptEndPosition)
                    {
                        break;
                    }
                    else
                    {
                        return;
                    }
                }
            case Qt::Key_Home:
                cursor.setPosition(mPromptEndPosition);
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
                handleTabKeyPressed();
                return;

            default:
                if (textCursor().selectionStart() < mPromptEndPosition)
                {
                    moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                }
                mInCompletion         = false;
                mCurrentHistoryIndex = -1;
        }
        QTextEdit::keyPressEvent(e);
    }

    void PythonConsole::mousePressEvent(QMouseEvent* event)
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

    void PythonConsole::insertAtEnd(const QString& text, QColor textColor)
    {
        moveCursor(QTextCursor::End);
        setTextColor(textColor);
        insertPlainText(QString(text));
    }

    void PythonConsole::handleStdout(const QString& output)
    {
        insertAtEnd(output, mStandardColor);
    }

    void PythonConsole::handleError(const QString& output)
    {
        QString append_out = output;
        if (!append_out.endsWith("\n"))
            append_out += "\n";
        insertAtEnd(append_out, mErrorColor);
    }

    void PythonConsole::clear()
    {
        QTextEdit::clear();
    }

    void PythonConsole::displayPrompt()
    {
        //QTextCursor cursor = textCursor();
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::End);
        //DEBUG SAVE FORMATS AS MEMBERS
        QTextCharFormat format;
        format.setForeground(mPromptColor);
        cursor.setCharFormat(format);
        if (mInCompoundPrompt)
        {
            cursor.insertText(mCompoundPrompt);
            if (mCompoundPromptEndPosition < 0)
            {
                mCompoundPromptEndPosition = mPromptEndPosition;
            }
        }
        else
        {
            cursor.insertText(mStandardPrompt);
            mCompoundPromptEndPosition = -1;
        }
        cursor.movePosition(QTextCursor::EndOfLine);
        setTextCursor(cursor);

        //    mPromptBlockNumber = textCursor().blockNumber();
        mPromptLength       = mStandardPrompt.length();
        mPromptEndPosition = textCursor().position();
    }

    void PythonConsole::interpretCommand()
    {
        QString input      = getCurrentCommand();
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n");
        if (!input.isEmpty())
        {
            //        gPythonContext->addHistory(input);
            mHistory->addHistory(input.toStdString());
        }
        if ((!mInCompoundPrompt && gPythonContext->checkCompleteStatement(input) != 0) || (mInCompoundPrompt && input.isEmpty() && gPythonContext->checkCompleteStatement(input) != 0))
        {
            mCurrentCompoundInput += input;
            if (mInCompoundPrompt)
            {
                gPythonContext->interpret(mCurrentCompoundInput, true);
            }
            else
            {
                gPythonContext->interpret(input, false);
            }
            mInCompoundPrompt     = false;
            mCurrentCompoundInput = "";
            displayPrompt();
        }
        else
        {
            mCurrentCompoundInput += input + "\n";
            mInCompoundPrompt = true;
            displayPrompt();
        }
        mHistory->updateFromFile();
        //qDebug() << mCurrentCompoundInput;
    }

    QString PythonConsole::getCurrentCommand()
    {
        QTextCursor cursor = textCursor();

        cursor.setPosition(mPromptEndPosition);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

        QString command = cursor.selectedText();
        cursor.clearSelection();
        return command;
    }

    void PythonConsole::replaceCurrentCommand(const QString& new_command)
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(mPromptEndPosition);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.insertText(new_command);
    }

    void PythonConsole::appendToCurrentCommand(const QString& appendix)
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(appendix);
    }

    bool PythonConsole::selectionEditable()
    {
        QTextCursor cursor(document());
        cursor.setPosition(textCursor().selectionStart());
        return false;
    }

    void PythonConsole::handleUpKeyPressed()
    {
        auto length = mHistory->size() - 1;

        if (mCurrentHistoryIndex == -1)
        {
            mCurrentInput         = getCurrentCommand();
            mCurrentHistoryIndex = length;
            replaceCurrentCommand(QString::fromStdString(mHistory->getHistoryItem(mCurrentHistoryIndex)));
        }
        else
        {
            if (mCurrentHistoryIndex == 0)
            {
                return;
            }

            mCurrentHistoryIndex--;
            replaceCurrentCommand(QString::fromStdString(mHistory->getHistoryItem(mCurrentHistoryIndex)));
        }
    }

    void PythonConsole::handleDownKeyPressed()
    {
        auto length = mHistory->size() - 1;

        if (mCurrentHistoryIndex == -1)
        {
            return;
        }

        if (mCurrentHistoryIndex == length)
        {
            mCurrentHistoryIndex = -1;
            replaceCurrentCommand(mCurrentInput);
        }
        else
        {
            mCurrentHistoryIndex++;
            replaceCurrentCommand(QString::fromStdString(mHistory->getHistoryItem(mCurrentHistoryIndex)));
        }
    }

    void PythonConsole::handleTabKeyPressed()
    {
        if (mInCompoundPrompt)
        {
            mCurrentInput = mCurrentCompoundInput + getCurrentCommand();
        }
        else
        {
            mCurrentInput = getCurrentCommand();
        }
        QString current_line = getCurrentCommand();
        if (current_line.isEmpty())
        {
            insertPlainText("\t");
        }
        else
        {
            log_info("python", "completing: '{}'", mCurrentInput.toStdString());
            auto r = gPythonContext->complete(mCurrentInput, true);
            if (r.size() == 1)
            {
                appendToCurrentCommand(QString::fromStdString(std::get<1>(r.at(0))));
            }
            else if (r.size() > 1)
            {
                mInCompletion         = true;
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
                handleStdout(candidates);
                displayPrompt();
                replaceCurrentCommand(current_line + matching_prefix);
            }
        }
    }
}

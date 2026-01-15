#include "gui/python/python_console.h"

#include "hal_core/utilities/log.h"
#include "gui/gui_globals.h"
#include "gui/hal_qt_compat/hal_qt_compat.h"

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include "gui/python/python_console_history.h"
#include "gui/python/python_console_qss_adapter.h"
#include "gui/main_window/main_window.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"

namespace hal
{
    PythonConsole::PythonConsole(QWidget* parent)
        : QTextEdit(parent), mStandardPrompt(">>> "), mCompoundPrompt("... "), mInputPrompt("==> "),
          mPromptBlockNumber(0), mPromptLength(0), mPromptEndPosition(0), mCompoundPromptEndPosition(0),
          mPromptType(Standard), mInCompletion(false), mCurrentCompoundInput(""), mCurrentInput(""), mCurrentHistoryIndex(-1), mCurrentCompleterIndex(0),
          mHistory(std::make_shared<PythonConsoleHistory>())
    {
        this->document()->setMaximumBlockCount(1000);
        setFrameStyle(QFrame::NoFrame);
        setUndoRedoEnabled(false);
        ensureCursorVisible();

        gPythonContext->setConsole(this);
        gPythonContext->interpretForeground("print(\"Python \" + sys.version)");
        gPythonContext->interpretForeground("print(sys.executable + \" on \" + sys.platform)");
        displayPrompt();
        mAbortThreadWidget = new PythonConsoleAbortThread(this);
        connect(MainWindow::sSettingStyle,&SettingsItemDropdown::intChanged,this,&PythonConsole::handleStyleChanged);
    }

    PythonConsole::~PythonConsole()
    {
        gPythonContext->setConsole(nullptr);
    }

    void PythonConsole::handleStyleChanged(int istyle)
    {
        Q_UNUSED(istyle);
        setHtml(PythonConsoleQssAdapter::instance()->updateStyle(toHtml()));
    }

    void PythonConsole::keyPressEventInputMode(QKeyEvent *e)
    {
        mCurrentHistoryIndex = -1;
        switch (e->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            QString input      = getCurrentCommand();
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText("\n");
            Q_EMIT inputReceived(input);
            break;
        }
        default:
            if (textCursor().selectionStart() < mPromptEndPosition)
            {
                moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            }
            mInCompletion         = false;
        }
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

        if (isInputMode())
        {
            keyPressEventInputMode(e);
        }
        else
        {
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
        insertAtEnd(output, PythonConsoleQssAdapter::instance()->standardColor());
    }

    void PythonConsole::handleError(const QString& output)
    {
        QString append_out = output;
        if (!append_out.endsWith("\n"))
            append_out += "\n";
        insertAtEnd(append_out, PythonConsoleQssAdapter::instance()->errorColor());
    }

    void PythonConsole::clear()
    {
        QTextEdit::clear();
    }

    void PythonConsole::setInputMode(bool state)
    {
        if (state)
            mPromptType = Input;
        else
            mPromptType = Standard;
    }

    void PythonConsole::displayPrompt()
    {
        //QTextCursor cursor = textCursor();
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::End);
        //DEBUG SAVE FORMATS AS MEMBERS
        QTextCharFormat format;
        format.setForeground(PythonConsoleQssAdapter::instance()->promtColor());
        cursor.setCharFormat(format);
        switch (mPromptType)
        {
        case Compound:
            cursor.insertText(mCompoundPrompt);
            if (mCompoundPromptEndPosition < 0)
            {
                mCompoundPromptEndPosition = mPromptEndPosition;
            }
            break;
        case Standard:
            cursor.insertText(mStandardPrompt);
            mCompoundPromptEndPosition = -1;
            break;
        case Input:
            cursor.insertText(mInputPrompt);
            mCompoundPromptEndPosition = -1;
            break;
        }
        cursor.movePosition(QTextCursor::EndOfLine);
        setTextCursor(cursor);

        //    mPromptBlockNumber = textCursor().blockNumber();
        mPromptLength       = mStandardPrompt.length();
        mPromptEndPosition = textCursor().position();
    }

    void PythonConsole::handleThreadFinished()
    {
        mAbortThreadWidget->stop();
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
        if ((!isCompound() && gPythonContext->checkCompleteStatement(input) != 0) || (isCompound() && input.isEmpty() && gPythonContext->checkCompleteStatement(input) != 0))
        {
            mCurrentCompoundInput += input;
            if (isCompound())
            {
                gPythonContext->interpretBackground(this, mCurrentCompoundInput, true);
            }
            else
            {
                gPythonContext->interpretBackground(this, input, false);
            }
            mPromptType     = Standard;
            mCurrentCompoundInput = "";
            mAbortThreadWidget->start();
        }
        else
        {
            mCurrentCompoundInput += input + "\n";
            mPromptType     = Compound;
            displayPrompt();
        }
        mHistory->updateFromFile();
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
        auto lastIndex = mHistory->size() - 1;
        if (lastIndex < 0)
        {
            // history empty
            return;
        }

        if (mCurrentHistoryIndex == -1)
        {
            mCurrentInput         = getCurrentCommand();
            mCurrentHistoryIndex = lastIndex;
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
        auto lastIndex = mHistory->size() - 1;

        if (mCurrentHistoryIndex == -1)
        {
            return;
        }

        if (mCurrentHistoryIndex == lastIndex)
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
        switch (mPromptType)
        {
        case Compound:
            mCurrentInput = mCurrentCompoundInput + getCurrentCommand();
            break;
        case Standard:
            mCurrentInput = getCurrentCommand();
            break;
        case Input:
            mCurrentInput += "\t";
            insertPlainText("\t");
            return;
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

    PythonConsoleAbortThread* PythonConsole::abortThreadWidget()
    {
        return mAbortThreadWidget;
    }

    //------------------------------
    PythonConsoleAbortThread::PythonConsoleAbortThread(QWidget* parent)
        : QFrame(parent), mCount(0)
    {
        setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        setLineWidth(2);
        QHBoxLayout* layout = new QHBoxLayout(this);
        QtCompat::setMarginWidth(layout,2);
        mLabel = new QLabel(this);
        layout->addWidget(mLabel);
        mAbortButton = new QPushButton("Abort", this);
        mAbortButton->setDisabled(true);
        mAbortButton->setMaximumWidth(270);
        connect(mAbortButton,&QPushButton::clicked,this,&PythonConsoleAbortThread::handleAbortButton);
        layout->addWidget(mAbortButton);
        mTimer = new QTimer(this);
        connect(mTimer,&QTimer::timeout,this,&PythonConsoleAbortThread::handleTimeout);
    }

    void  PythonConsoleAbortThread::handleAbortButton()
    {
        gPythonContext->abortThread();
        log_info("gui", "Python console command execution aborted by user");
    }


    void PythonConsoleAbortThread::handleTimeout()
    {
        if (isVisible() && !gPythonContext->isThreadRunning())
        {
            stop();
            return;
        }
        ++mCount;
        mLabel->setText(QString("Python interpreter running for %1 seconds").arg(mCount));
        if (mCount > 5)
        {
            mAbortButton->setEnabled(true);
            show();
        }
    }

    void PythonConsoleAbortThread::start()
    {
        mCount = 0;
        mTimer->start(1000);
        mAbortButton->setDisabled(true);
        hide();
    }

    void PythonConsoleAbortThread::stop()
    {
        mTimer->stop();
        mAbortButton->setDisabled(true);
        hide();
    }

}

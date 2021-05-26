//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "gui/python/python_context_subscriber.h"

#include <QTextEdit>
#include <memory>

namespace hal
{
    class PythonConsoleHistory;

    /**
     * @ingroup python-console
     * @brief An interactable python console.
     *
     * A widget that represents a simple python console with various features that most python consoles provide.
     */
    class PythonConsole : public QTextEdit, public PythonContextSubscriber
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         */
        PythonConsole(QWidget* parent = nullptr);

        /**
         * Overrides QTextEdit::keyPressEvent. It is used to handle the following (default) inputs: <br>
         * @li Ctrl+C: Copy
         * @li Ctrl+X: Cut
         * @li ENTER/RETURN: Interpret the written python command
         * @li BACKSPACE: Backspace
         * @li KEY_UP: Get the previous command from the history
         * @li KEY_DOWN: Get the next command from the history
         * @li TAB: Indent or autofill
         *
         * @param e - The key event.
         */
        void keyPressEvent(QKeyEvent* e) override;

        /**
         * Overrides QTextEdit::mouseEvent.
         *
         * @param event - The mouse event
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * Prints forwarded standard output messages.
         *
         * @param output - The forwarded output string.
         */
        virtual void handleStdout(const QString& output) override;

        /**
         * Prints forwarded error output messages.
         *
         * @param output - The forwarded output string.
         */
        virtual void handleError(const QString& output) override;

        /**
         * Clears the console.
         */
        virtual void clear() override;

        /**
         * Used to display the editors content within the QTextEdit.
         */
        void displayPrompt();

        /**
         * Process the recently typed python command after pressing ENTER/RETURN. If the command is within (or starts) a
         * compound statement (for example a 'for'-loop), the command is appended to the current compound input. Else, the
         * command (or the finished compound) is interpreted and executed.
         */
        void interpretCommand();

        /**
         * Get the recently types python command (of the current line).
         *
         * @returns the current command as a string.
         */
        QString getCurrentCommand();

        /**
         * Replace the recently types python command with another one. (Used e.g. for navigating in the history)
         *
         * @param new_command - The new command.
         */
        void replaceCurrentCommand(const QString& new_command);

        /**
         * Append the recently types python command with a string. (Used for auto-completion)
         *
         * @param new_command - The string that should be appended.
         */
        void appendToCurrentCommand(const QString& new_command);

        /**
         * Currently unused.
         *
         * @returns false
         */
        bool selectionEditable();

        /**
         * Handles presses of KEY_UP. Used to jump back in history.
         */
        void handleUpKeyPressed();

        /**
         * Handles presses of KEY_DOWN. Used to jump forward in history.
         */
        void handleDownKeyPressed();

        /**
         * Handles presses of TAB. Used to indent or auto-complete the current command.
         */
        void handleTabKeyPressed();

        /**
         * Append the console with a text of a certain color.
         *
         * @param text - The appended text.
         * @param textColor - The color of the appended text.
         */
        void insertAtEnd(const QString& text, QColor textColor);

    private:
        QColor mPromptColor;
        QColor mStandardColor;
        QColor mErrorColor;

        QString mStandardPrompt;
        QString mCompoundPrompt;

        int mPromptBlockNumber;
        int mPromptLength;
        int mPromptEndPosition;
        int mCompoundPromptEndPosition;

        bool mInCompoundPrompt;
        bool mInCompletion;

        QString mCurrentCompoundInput;
        QString mCurrentInput;

        int mCurrentHistoryIndex;
        int mCurrentCompleterIndex;

        std::shared_ptr<PythonConsoleHistory> mHistory;
    };
}

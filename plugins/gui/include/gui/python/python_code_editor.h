//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "gui/code_editor/code_editor.h"

#include <QUuid>

namespace hal
{
    /**
     * A Code Editor that is extended to be used as a python editor. <br>
     * The editor can be used to work on existing python files and provides functionality to stay in sync with the
     * original file. <br>
     * Moreover it provides python specific tools for improving the coding experience like auto indentation and
     * auto completion (using the PythonContext).
     *
     */
    class PythonCodeEditor : public CodeEditor
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        PythonCodeEditor(QWidget* parent = nullptr);

        /**
         * Get the absolute path of the file the editor works with.
         * Returns an empty string, if the editor is not connected to a file (i.e. a new file).
         *
         * @returns the absolute path of the file. Empty string if no file path is set.
         */
        QString getFileName();

         /**
          * Set the absolute path of the file the editor works with.
          *
          * @param name - The absolute file path
          */
        void set_file_name(const QString name);

        /**
         * If called with <b>true</b>:
         * Mark that the file, the editor works with, was modified from outside (e.g. by editing it in another program)
         * and the user has to be informed.
         * If called with <b>false</b>:
         * The user has been informed and has already selected further actions (like reload the editor or ignore the changes).
         * Therefore the file can be assumed to be in sync with the editor again.
         *
         * @param base_file_modified - If the base file is in sync with the editor (see description)
         */
        void setBaseFileModified(bool base_file_modified);

        /**
         * If <b>true</b>:
         * The file, the editor works with, was modified from outside (e.g. by editing it in another program)
         * and the user has to be informed.
         * If <b>false</b>:
         * The base file is assumed to be in sync with the editor.
         */
        bool isBaseFileModified();

        /**
         * Get the unique id of this editor.
         *
         * @returns the Uuid of the editor
         */
        QUuid getUuid() const;

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted after a key was pressed within the code editor. <br>
         * Emitted by PythonCodeEditor::keyPressEvent.
         *
         * @param e - The key event
         */
        void keyPressed(QKeyEvent* e);

    private:
        /**
         * Handle key presses within the python code editor. <br>
         * Used to handle the auto indent and the autofill logic. <br>
         * Overridden of QPlainTextEdit::keyPressEvent.
         * Emits PythonCodeEditor::keyPressed (at the beginning)
         *
         * @param e - The key event
         */
        void keyPressEvent(QKeyEvent* e) override;

        /**
         * Special handling of keystrokes of TAB. <br>
         * It is used to indent the currently selected lines or handle auto completion (based on the cursor position).
         */
        void handleTabKeyPressed();

        /**
         * Special handling of keystrokes of SHIFT+TAB. <br>
         * Used to remove indentations of the currently selected lines.
         */
        void handleShiftTabKeyPressed();

        /**
         * Special handling of keystrokes of RETURN. <br>
         * Used to indent new lines in python style (After a line ending with ':').
         */
        void handleReturnKeyPressed();

        /**
         * Special handling of keystrokes of BACKSPACE. <br>
         */
        void handleBackspaceKeyPressed(QKeyEvent* e);

        /**
         * Special handling of keystrokes of DELETE. <br>
         */
        void handleDeleteKeyPressed(QKeyEvent* e);

        /**
         * Special handling of keystrokes of INSERT. <br>
         * Used to toggle the overwrite mode.
         */
        void handleInsertKeyPressed();

        /**
         * Slot to handle the redo shortcut.
         */
        void handleRedoRequested();

        /**
         * If <b>true</b>: <br>
         * Indent the currently selected lines with one tab. <br>
         * If <b>false</b>: <br>
         * Remove existing indentations in the currently selected lines (one tab per line). <br> <br>
         *
         * It initializes auto completion instead, if the cursor is not at the beginning of the (written) line.
         *
         * @param indentUnindent - True to indent. False to unindent.
         */
        void indentSelection(bool indentUnindent);

        /**
         * Helper function to decide how many characters has to be added (deleted) for the next indentation (unindentation).
         *
         * @param indentUnindent - True to compute for indentation. False to compute for unindentation.
         * @param current_indent - The current indentation (amount of ' ' characters)
         * @returns the number of spaces that has to be added/deleted for the next indentation/unindentation
         */
        int nextIndent(bool indentUnindent, int current_indent);

        /**
         * Opens the auto completion menu.
         */
        void handleAutocomplete();

        /**
         * Inserts the chosen auto completion at the cursor position.
         *
         * @param completion - A tuple (name_with_symbols, completion). The field name_with_symbols will be ignored.
         */
        void performCodeCompletion(std::tuple<std::string, std::string> completion);



        QString mFileName;
        QString mTextState;

        bool mBaseFileModified;

        QUuid mUuid;
    };
}

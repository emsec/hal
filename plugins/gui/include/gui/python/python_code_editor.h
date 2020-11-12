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
    class PythonCodeEditor : public CodeEditor
    {
        Q_OBJECT
    public:
        PythonCodeEditor(QWidget* parent = nullptr);

        QString getFileName();
        void set_file_name(const QString name);

        void setBaseFileModified(bool base_file_modified);
        bool isBaseFileModified();

        QUuid getUuid() const;

    Q_SIGNALS:
        void keyPressed(QKeyEvent* e);

    private:
        void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
        void handleTabKeyPressed();
        void handleShiftTabKeyPressed();
        void handleReturnKeyPressed();
        void handleBackspaceKeyPressed(QKeyEvent* e);
        void handleDeleteKeyPressed(QKeyEvent* e);
        void handleInsertKeyPressed();

        void handleRedoRequested();

        void indentSelection(bool indentUnindent);
        int nextIndent(bool indentUnindent, int current_indent);

        void handleAutocomplete();
        void performCodeCompletion(std::tuple<std::string, std::string> completion);

        QString mFileName;
        QString mTextState;

        bool mBaseFileModified;

        QUuid mUuid;
    };
}

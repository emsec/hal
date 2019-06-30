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

#ifndef PYTHON_CODE_EDITOR_H
#define PYTHON_CODE_EDITOR_H

#include "code_editor/code_editor.h"

class python_code_editor : public code_editor
{
    Q_OBJECT
public:
    python_code_editor(QWidget* parent = nullptr);

    QString get_file_name();
    void set_file_name(const QString name);

<<<<<<< HEAD
    void set_base_file_modified(bool base_file_modified);
    bool is_base_file_modified();
=======
Q_SIGNALS:
    void key_pressed(QKeyEvent* e);
>>>>>>> master

private:
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
    void handle_tab_key_pressed();
    void handle_shift_tab_key_pressed();
    void handle_return_key_pressed();
    void handle_backspace_key_pressed(QKeyEvent* e);
    void handle_delete_key_pressed(QKeyEvent* e);
    void handle_insert_key_pressed();

    void handle_redo_requested();

    void indent_selection(bool indentUnindent);
    int next_indent(bool indentUnindent, int current_indent);

    void handle_autocomplete();
    void perform_code_completion(std::tuple<std::string, std::string> completion);

    QString m_file_name;
    QString m_text_state;

    bool m_base_file_modified;
};

#endif //PYTHON_CODE_EDITOR_H

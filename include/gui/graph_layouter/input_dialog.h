//  MIT License
//
//  Copyright (c) 2019 Marc Fyrbiak
//  Copyright (c) 2019 Sebastian Wallat
//  Copyright (c) 2019 Max Hoffmann
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

#ifndef input_dialog_H
#define input_dialog_H

#include "gui_graph_gate.h"
#include "gui_graph_net.h"
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class input_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit input_dialog(QWidget* parent = 0, QString query = "Input:", QString init = "");
    ~input_dialog() = default;

    QString get_text() const;

private:
    QLabel* label_name;
    QLineEdit* edit_name;
    QGridLayout* layout;
    QPushButton* save_button;
    QPushButton* cancel_button;

    void init(QString query, QString init);
};

#endif    //input_dialog_H

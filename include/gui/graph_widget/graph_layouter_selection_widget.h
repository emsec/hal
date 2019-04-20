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

#ifndef GRAPH_LAYOUTER_SELECTION_WIDGET_H
#define GRAPH_LAYOUTER_SELECTION_WIDGET_H

#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class graph_widget;

class graph_layouter_selection_widget : public QWidget
{
    Q_OBJECT

public:
    explicit graph_layouter_selection_widget(graph_widget* widget, QWidget* parent = 0);

public Q_SLOTS:
    void ok_clicked(bool checked);
    void cancel_clicked(bool checked);

    void current_selection_changed(QString layouter);

private:
    QHBoxLayout* m_horizontal_layout;
    QVBoxLayout* m_vertical_layout;
    QHBoxLayout* m_button_layout;

    QListWidget* m_list;
    QTextEdit* m_description;

    QPushButton* m_ok;
    QPushButton* m_cancel;

    graph_widget* m_widget;
};

#endif    // GRAPH_LAYOUTER_SELECTION_WIDGET_H

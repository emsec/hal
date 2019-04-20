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

#ifndef MODULE_DETAILS_WIDGET_H
#define MODULE_DETAILS_WIDGET_H

#include "def.h"
#include "graph_relay/graph_relay.h"
#include <QWidget>

class QVBoxLayout;
class QLabel;
class module;

class module_details_widget : public QWidget
{
    Q_OBJECT
public:
    module_details_widget(QWidget* parent = 0);

    void update(u32 module_id);

public Q_SLOTS:
    void handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data);

private:
    QVBoxLayout* m_content_layout;
    QLabel* m_label;

    u32 m_current_id;
};

#endif

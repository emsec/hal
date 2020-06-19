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

#include "def.h"
#include "selection_history_navigator/selection.h"

#include <QObject>
#include <list>
namespace hal{
class selection_history_navigator : public QObject
{
    Q_OBJECT

public:
    selection_history_navigator(unsigned int max_history_size, QObject* parent = 0);
    ~selection_history_navigator();

    void navigate_to_prev_item();
    void navigate_to_next_item();

    void set_max_history_size(unsigned int max_size);
    u32 get_max_history_size() const;

Q_SIGNALS:

public Q_SLOTS:
    void handle_selection_changed(void* sender);

private:
    unsigned int m_max_history_size;

    std::list<selection> m_selection_container;

    std::list<selection>::iterator m_current_item_iterator;

    void relay_selection(selection selection);

    void store_selection(u32 id, selection_relay::item_type type);
};
}

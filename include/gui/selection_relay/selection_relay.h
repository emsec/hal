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

#ifndef SELECTION_RELAY_H
#define SELECTION_RELAY_H

#include "def.h"
#include <QList>
#include <QObject>
#include <QPair>

class selection_relay : public QObject
{
    Q_OBJECT

public:
    enum class single_type
    {
        none = 0,
        gate = 1,
        net  = 2
    };

    enum class group_type
    {
        none = 0,
        gate = 1,
        net  = 2,
        both = 3
    };

    enum class subfocus
    {
        left   = 0,
        right  = 1,
        up     = 2,
        down   = 3,
        center = 5,
    };

    enum class Mode
    {
        override = 0,
        add      = 1,
        remove   = 2
    };

    explicit selection_relay(QObject* parent = 0);

    void register_sender(void* sender, QString name);
    void remove_sender(void* sender);

    void relay_gate_selection(void* sender, QList<u32>& gate_ids, Mode mode = Mode::override);
    void relay_net_selection(void* sender, QList<u32>& net_ids, Mode mode = Mode::override);
    void relay_module_selection(void* sender, QList<u32>& module_ids, Mode mode = Mode::override);
    void relay_combined_selection(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode = Mode::override);

    void relay_current_gate(void* sender, u32 gate_id);
    void relay_current_net(void* sender, u32 net_id);
    void relay_current_module(void* sender, u32 module_id);
    void relay_current_cleared(void* sender);
    void relay_current_deleted(void* sender);

    void relay_jump_gate(void* sender, u32 gate_id);
    void relay_jump_net(void* sender, u32 net_id);
    void relay_jump_module(void* sender, u32 module_id);
    void relay_jump_selection(void* sender);

    void relay_gate_highlight(void* sender, QList<u32>& ids, Mode mode = Mode::override, u32 channel = 0);
    void relay_net_highlight(void* sender, QList<u32>& ids, Mode mode = Mode::override, u32 channel = 0);
    void relay_module_highlight(void* sender, QList<u32>& ids, Mode mode = Mode::override, u32 channel = 0);
    void relay_combined_highlight(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode = Mode::override, u32 channel = 0);

Q_SIGNALS:
    void gate_selection_update(void* sender, const QList<u32>& gate_ids, Mode mode);
    void net_selection_update(void* sender, const QList<u32>& net_ids, Mode mode);
    void module_selection_update(void* sender, const QList<u32>& module_ids, Mode mode);
    void combined_selection_update(void* sender, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids, Mode mode);

    void current_gate_update(void* sender, u32 id);
    void current_net_update(void* sender, u32 id);
    void current_module_update(void* sender, u32 id);
    void current_cleared_update(void* sender);
    void current_deleted_update(void* sender);

    void jump_gate_update(void* sender, u32 id);
    void jump_net_update(void* sender, u32 id);
    void jump_module_update(void* sender, u32 id);
    void jump_selection_update(void* sender);

    void gate_highlight_update(void* sender, QList<u32>& ids, Mode mode, u32 channel);
    void net_highlight_update(void* sender, QList<u32>& ids, Mode mode, u32 channel);
    void module_highlight_update(void* sender, QList<u32>& ids, Mode mode, u32 channel);
    void combined_highlight_update(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode, u32 channel);

private:
    group_type m_selection_type;
    QList<u32> m_selected_net_ids;
    QList<u32> m_selected_gate_ids;
    QList<u32> m_selected_module_ids;

    single_type m_current_type;
    u32 m_current_net_id;
    u32 m_current_gate_id;
    u32 m_current_module_id;

    single_type m_focus_type;
    u32 m_focus_net_id;
    u32 m_focus_gate_id;
    u32 m_focus_module_id;
    subfocus m_subfocus;

    QList<QPair<void*, QString>> m_sender_register;
};

#endif    // SELECTION_RELAY_H

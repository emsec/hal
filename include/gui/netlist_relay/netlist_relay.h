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

#ifndef NETLIST_RELAY_H
#define NETLIST_RELAY_H

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"
#include "netlist/event_system/module_event_handler.h"

#include <QMap>
#include <QObject>

class module_item;
class module_model;

class netlist_relay : public QObject
{
    Q_OBJECT

public:
    explicit netlist_relay(QObject* parent = nullptr);
    ~netlist_relay();

    void register_callbacks();

    QColor get_module_color(const u32 id);

    module_item* get_module_item(const u32 id);
    module_model* get_module_model();

    void debug_change_module_name(const u32 id);
    void debug_change_module_color(const u32 id);
    void debug_add_selection_to_module(const u32 id);
    void debug_add_child_module(const u32 id);

Q_SIGNALS:
    // PROBABLY OBSOLETE
    void netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data) const;
    void module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data) const;
    void gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data) const;
    void net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data) const;

    void netlist_id_changed(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_input_filename_changed(const std::shared_ptr<netlist> n) const;
    void netlist_design_name_changed(const std::shared_ptr<netlist> n) const;
    void netlist_device_name_changed(const std::shared_ptr<netlist> n) const;
    void netlist_marked_global_vcc(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_marked_global_gnd(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_unmarked_global_vcc(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_unmarked_global_gnd(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_marked_global_input(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_marked_global_output(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_marked_global_inout(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_unmarked_global_input(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_unmarked_global_output(const std::shared_ptr<netlist> n, const u32 associated_data) const;
    void netlist_unmarked_global_inout(const std::shared_ptr<netlist> n, const u32 associated_data) const;

    void module_created(const std::shared_ptr<module> m) const;
    void module_removed(const std::shared_ptr<module> m) const;
    void module_name_changed(const std::shared_ptr<module> m) const;
    void module_parent_changed(const std::shared_ptr<module> m) const;
    void module_submodule_added(const std::shared_ptr<module> m, const u32 added_module) const;
    void module_submodule_removed(const std::shared_ptr<module> m, const u32 removed_module) const;
    void module_gate_assigned(const std::shared_ptr<module> m, const u32 assigned_gate) const;
    void module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate) const;

    void gate_created(const std::shared_ptr<gate> g) const;
    void gate_removed(const std::shared_ptr<gate> g) const;
    void gate_name_changed(const std::shared_ptr<gate> g) const;

    void net_created(const std::shared_ptr<net> n) const;
    void net_removed(const std::shared_ptr<net> n) const;
    void net_name_changed(const std::shared_ptr<net> n) const;
    void net_src_changed(const std::shared_ptr<net> n) const;
    void net_dst_added(const std::shared_ptr<net> n, const u32 dst_gate_id) const;
    void net_dst_removed(const std::shared_ptr<net> n, const u32 dst_gate_id) const;

    // GUI
    void module_color_changed(const std::shared_ptr<module> m) const;

private Q_SLOTS:
    void debug_handle_file_opened();
    void debug_handle_file_closed();

private:
    void relay_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data);
    void relay_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data);
    void relay_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data);
    void relay_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data);

    QMap<u32, QColor> m_module_colors;

    QMap<u32, QString> m_gate_aliases;
    QMap<u32, QString> m_net_aliases;

    QMap<u32, module_item*> m_module_items;
    module_model* m_module_model;
};

#endif // NETLIST_RELAY_H

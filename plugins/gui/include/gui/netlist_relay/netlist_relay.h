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

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"
#include "netlist/event_system/module_event_handler.h"

#include <QMap>
#include <QObject>

namespace hal
{
    class ModuleItem;
    class ModuleModel;
    class Module;

    class NetlistRelay : public QObject
    {
        Q_OBJECT

    public:
        explicit NetlistRelay(QObject* parent = nullptr);
        ~NetlistRelay();

        void register_callbacks();

        QColor get_module_color(const u32 id);

        ModuleModel* get_ModuleModel();

        void debug_change_module_name(const u32 id);
        void debug_change_module_type(const u32 id);
        void debug_change_module_color(const u32 id);
        void debug_add_selection_to_module(const u32 id);
        void debug_add_child_module(const u32 id);
        void debug_delete_module(const u32 id);

    Q_SIGNALS:
        // PROBABLY OBSOLETE
        void netlist_event(netlist_event_handler::event ev, Netlist* object, u32 associated_data) const;
        void module_event(module_event_handler::event ev, Module* object, u32 associated_data) const;
        void gate_event(gate_event_handler::event ev, Gate* object, u32 associated_data) const;
        void net_event(net_event_handler::event ev, Net* object, u32 associated_data) const;

        void netlist_id_changed(Netlist* n, const u32 associated_data) const;
        void netlist_input_filename_changed(Netlist* n) const;
        void netlist_design_name_changed(Netlist* n) const;
        void netlist_device_name_changed(Netlist* n) const;
        void netlist_marked_global_vcc(Netlist* n, const u32 associated_data) const;
        void netlist_marked_global_gnd(Netlist* n, const u32 associated_data) const;
        void netlist_unmarked_global_vcc(Netlist* n, const u32 associated_data) const;
        void netlist_unmarked_global_gnd(Netlist* n, const u32 associated_data) const;
        void netlist_marked_global_input(Netlist* n, const u32 associated_data) const;
        void netlist_marked_global_output(Netlist* n, const u32 associated_data) const;
        void netlist_marked_global_inout(Netlist* n, const u32 associated_data) const;
        void netlist_unmarked_global_input(Netlist* n, const u32 associated_data) const;
        void netlist_unmarked_global_output(Netlist* n, const u32 associated_data) const;
        void netlist_unmarked_global_inout(Netlist* n, const u32 associated_data) const;

        void module_created(Module* m) const;
        void module_removed(Module* m) const;
        void module_name_changed(Module* m) const;
        void module_parent_changed(Module* m) const;
        void module_submodule_added(Module* m, const u32 added_module) const;
        void module_submodule_removed(Module* m, const u32 removed_module) const;
        void module_gate_assigned(Module* m, const u32 assigned_gate) const;
        void module_gate_removed(Module* m, const u32 removed_gate) const;
        void module_input_port_name_changed(Module* m, const u32 respective_net) const;
        void module_output_port_name_changed(Module* m, const u32 respective_net) const;
        void module_type_changed(Module* m) const;

        void gate_created(Gate* g) const;
        void gate_removed(Gate* g) const;
        void gate_name_changed(Gate* g) const;

        void net_created(Net* n) const;
        void net_removed(Net* n) const;
        void net_name_changed(Net* n) const;
        void net_source_added(Net* n, const u32 dst_gate_id) const;
        void net_source_removed(Net* n, const u32 dst_gate_id) const;
        void net_destination_added(Net* n, const u32 dst_gate_id) const;
        void net_destination_removed(Net* n, const u32 dst_gate_id) const;

        // GUI
        void module_color_changed(Module* m) const;

    private Q_SLOTS:
        void debug_handle_file_opened();
        void debug_handle_file_closed();

    private:
        void relay_netlist_event(netlist_event_handler::event ev, Netlist* object, u32 associated_data);
        void relay_module_event(module_event_handler::event ev, Module* object, u32 associated_data);
        void relay_gate_event(gate_event_handler::event ev, Gate* object, u32 associated_data);
        void relay_net_event(net_event_handler::event ev, Net* object, u32 associated_data);

        QMap<u32, QColor> m_module_colors;

        QMap<u32, QString> m_gate_aliases;
        QMap<u32, QString> m_net_aliases;

        ModuleModel* m_ModuleModel;
    };
}

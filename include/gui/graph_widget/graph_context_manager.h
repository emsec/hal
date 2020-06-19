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

#include <QObject>
#include <QStringList>
#include <QVector>

namespace hal
{
    class Gate;
    class Module;
    class Net;

    class graph_context;
    class graph_layouter;
    class graph_shader;
    class graph_context;

    class graph_context_manager : public QObject
    {
        Q_OBJECT

    public:
        graph_context_manager();

        graph_context* create_new_context(const QString& name);
        void rename_graph_context(graph_context* ctx, const QString& new_name);
        void delete_graph_context(graph_context* ctx);
        QVector<graph_context*> get_contexts() const;
        bool context_with_name_exists(const QString& name) const;

        //void handle_module_created(const std::shared_ptr<Module> m) const;
        void handle_module_removed(const std::shared_ptr<Module> m);
        void handle_module_name_changed(const std::shared_ptr<Module> m) const;
        void handle_module_color_changed(const std::shared_ptr<Module> m) const;
        //void handle_module_parent_changed(const std::shared_ptr<Module> m) const;
        void handle_module_submodule_added(const std::shared_ptr<Module> m, const u32 added_module) const;
        void handle_module_submodule_removed(const std::shared_ptr<Module> m, const u32 removed_module);
        void handle_module_gate_assigned(const std::shared_ptr<Module> m, const u32 inserted_gate) const;
        void handle_module_gate_removed(const std::shared_ptr<Module> m, const u32 removed_gate);

        //void handle_gate_created(const std::shared_ptr<Gate> g) const;
        //void handle_gate_removed(const std::shared_ptr<Gate> g) const;
        void handle_gate_name_changed(const std::shared_ptr<Gate> g) const;

        void handle_net_created(const std::shared_ptr<Net> n) const;
        void handle_net_removed(const std::shared_ptr<Net> n) const;
        void handle_net_name_changed(const std::shared_ptr<Net> n) const;
        void handle_net_source_added(const std::shared_ptr<Net> n, const u32 src_gate_id) const;
        void handle_net_source_removed(const std::shared_ptr<Net> n, const u32 src_gate_id) const;
        void handle_net_destination_added(const std::shared_ptr<Net> n, const u32 dst_gate_id) const;
        void handle_net_destination_removed(const std::shared_ptr<Net> n, const u32 dst_gate_id) const;
        void handle_marked_global_input(u32 net_id);
        void handle_marked_global_output(u32 net_id);
        void handle_unmarked_global_input(u32 net_id);
        void handle_unmarked_global_output(u32 net_id);

        graph_layouter* get_default_layouter(graph_context* const context) const;
        graph_shader* get_default_shader(graph_context* const context) const;

    Q_SIGNALS:
        void context_created(graph_context* context);
        void context_renamed(graph_context* context);
        void deleting_context(graph_context* context);

    private:
        QVector<graph_context*> m_graph_contexts;
    };
}

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

    class GraphContext;
    class GraphLayouter;
    class GraphShader;
    class GraphContext;

    class ContextTableModel;

    class GraphContextManager : public QObject
    {
        Q_OBJECT

    public:
        GraphContextManager();

        GraphContext* create_new_context(const QString& name);
        void rename_graph_context(GraphContext* ctx, const QString& new_name);
        void delete_graph_context(GraphContext* ctx);
        QVector<GraphContext*> get_contexts() const;
        bool context_with_name_exists(const QString& name) const;

        //void handle_module_created(Module* m) const;
        void handle_module_removed(Module* m);
        void handle_module_name_changed(Module* m) const;
        void handle_module_color_changed(Module* m) const;
        //void handle_module_parent_changed(Module* m) const;
        void handle_module_submodule_added(Module* m, const u32 added_module) const;
        void handle_module_submodule_removed(Module* m, const u32 removed_module);
        void handle_module_gate_assigned(Module* m, const u32 inserted_gate) const;
        void handle_module_gate_removed(Module* m, const u32 removed_gate);

        //void handle_gate_created(Gate* g) const;
        //void handle_gate_removed(Gate* g) const;
        void handle_gate_name_changed(Gate* g) const;

        void handle_net_created(Net* n) const;
        void handle_net_removed(Net* n) const;
        void handle_net_name_changed(Net* n) const;
        void handle_net_source_added(Net* n, const u32 src_gate_id) const;
        void handle_net_source_removed(Net* n, const u32 src_gate_id) const;
        void handle_net_destination_added(Net* n, const u32 dst_gate_id) const;
        void handle_net_destination_removed(Net* n, const u32 dst_gate_id) const;
        void handle_marked_global_input(u32 net_id);
        void handle_marked_global_output(u32 net_id);
        void handle_unmarked_global_input(u32 net_id);
        void handle_unmarked_global_output(u32 net_id);

        GraphLayouter* get_default_layouter(GraphContext* const context) const;
        GraphShader* get_default_shader(GraphContext* const context) const;

        ContextTableModel* get_context_table_model() const;

    Q_SIGNALS:
        void context_created(GraphContext* context);
        void context_renamed(GraphContext* context);
        void deleting_context(GraphContext* context);

    private:
        QVector<GraphContext*> m_graph_contexts;

        ContextTableModel* m_context_table_model;
    };
}

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

#include "hal_core/defines.h"

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

        GraphContext* createNewContext(const QString& name);
        void renameGraphContext(GraphContext* ctx, const QString& new_name);
        void deleteGraphContext(GraphContext* ctx);
        QVector<GraphContext*> getContexts() const;
        bool contextWithNameExists(const QString& name) const;

        //void handle_module_created(Module* m) const;
        void handleModuleRemoved(Module* m);
        void handleModuleNameChanged(Module* m) const;
        void handleModuleTypeChanged(Module* m) const;
        void handleModuleColorChanged(Module* m) const;
        //void handle_module_parent_changed(Module* m) const;
        void handleModuleSubmoduleAdded(Module* m, const u32 added_module) const;
        void handleModuleSubmoduleRemoved(Module* m, const u32 removed_module);
        void handleModuleGateAssigned(Module* m, const u32 inserted_gate) const;
        void handleModuleGateRemoved(Module* m, const u32 removed_gate);
        void handleModuleInputPortNameChanged(Module* m, const u32 net);
        void handleModuleOutputPortNameChanged(Module* m, const u32 net);

        //void handle_gate_created(Gate* g) const;
        //void handleGateRemoved(Gate* g) const;
        void handleGateNameChanged(Gate* g) const;

        void handleNetCreated(Net* n) const;
        void handleNetRemoved(Net* n) const;
        void handleNetNameChanged(Net* n) const;
        void handleNetSourceAdded(Net* n, const u32 src_gate_id) const;
        void handleNetSourceRemoved(Net* n, const u32 src_gate_id) const;
        void handleNetDestinationAdded(Net* n, const u32 dst_gate_id) const;
        void handleNetDestinationRemoved(Net* n, const u32 dst_gate_id) const;
        void handleMarkedGlobalInput(u32 mNetId);
        void handleMarkedGlobalOutput(u32 mNetId);
        void handleUnmarkedGlobalInput(u32 mNetId);
        void handleUnmarkedGlobalOutput(u32 mNetId);

        GraphLayouter* getDefaultLayouter(GraphContext* const context) const;
        GraphShader* getDefaultShader(GraphContext* const context) const;

        ContextTableModel* getContextTableModel() const;

    Q_SIGNALS:
        void contextCreated(GraphContext* context);
        void contextRenamed(GraphContext* context);
        void deletingContext(GraphContext* context);

    private:
        QVector<GraphContext*> mGraphContexts;

        ContextTableModel* mContextTableModel;
    };
}

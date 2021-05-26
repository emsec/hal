//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

    class SettingsItemCheckbox;
    /**
     * @ingroup graph-contexts
     * @brief User interface to manage all GraphContext%s.
     *
     * The GraphContextManager is a manager class that contains and manages all GraphContext%s. <br>
     * It can be used to create, delete or rename GraphContext%s.  <br>
     * Moreover it is responsible for applying changes of the Netlist to the contexts. Therefore it inspects which
     * contexts are affected by the corresponding changes to only adjust the relevant contexts.
     */
    class GraphContextManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         */
        GraphContextManager();

        /**
         * Creates a new context with the given name. The context is initialized with the default GraphLayouter and
         * GraphShader. <br>
         * Emits the signal contextCreated.
         *
         * @param name - The name of the new context. Names don't have to be unique.
         * @returns the created GraphContext
         */
        GraphContext* createNewContext(const QString& name);

        /**
         * Renames a GraphContext. <br>
         * Emits the signal contextRenamed.
         *
         * @param ctx - The graph context to rename. Must not be a <i>nullptr</i>.
         * @param newName - The new name of the context
         */
        void renameGraphContextAction(GraphContext* ctx, const QString& newName);

        /**
         * Removes and deletes the given GraphContext. The passed pointer will be a <i>nullptr</i> afterwards.<br>
         * Emits deletingContext before the deletion.
         *
         * @param ctx - The graph context to delete.
         */
        void deleteGraphContext(GraphContext* ctx);

        /**
         * Gets a list of all current GraphContext%s.
         *
         * @returns a list of all GrapgContext%s.
         */
        QVector<GraphContext*> getContexts() const;
        GraphContext* getCleanContext(const QString& name) const;
        GraphContext* getContextById(u32 id) const;

        /**
         * Checks if a context with the given name exists.
         *
         * @param name - The context name
         * @returns <b>true</b> if a context with the name exists
         */
        bool contextWithNameExists(const QString& name) const;

        //void handle_module_created(Module* m) const;
        /**
         * Handler to be called after a module has been removed. Used to apply the changes in the affected contexts.<br>
         * The module is already removed from the netlist at this point. However the module isn't deleted yet (not <i>nullptr</i>).
         *
         * @param m - The module that has been removed
         */
        void handleModuleRemoved(Module* m);

        /**
         * Handler to be called after a module's name has been changed. Used to apply the changes in the affected contexts.
         *
         * @param m - The module that has been changed
         */
        void handleModuleNameChanged(Module* m) const;

        /**
         * Handler to be called after a module's type has been changed. Used to apply the changes in the affected contexts.
         *
         * @param m - The module that has been changed
         */
        void handleModuleTypeChanged(Module* m) const;

        /**
         * Handler to be called after a module's color has been changed. Used to apply the changes in the affected contexts.
         *
         * @param m - The module which color has been changed
         */
        void handleModuleColorChanged(Module* m) const;
        //void handle_module_parent_changed(Module* m) const;
        /**
         * Handler to be called after a submodule was added to a module. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param m - The module with the new submodule
         * @param added_module - The id of the added submodule
         */
        void handleModuleSubmoduleAdded(Module* m, const u32 added_module) const;

        /**
         * Handler to be called after a submodule was removed from a module. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param m - The module with the removed submodule
         * @param removed_module - The id of the removed submodule
         */
        void handleModuleSubmoduleRemoved(Module* m, const u32 removed_module);

        /**
         * Handler to be called after a gate was newly assigned to a module. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param m - The module with the new gate
         * @param inserted_gate - The id of the newly assigned gate
         */
        void handleModuleGateAssigned(Module* m, const u32 inserted_gate) const;

        /**
         * Handler to be called after a gate was removed from a module. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param m - The module with the removed gate
         * @param removed_gate - The id of the removed gate
         */
        void handleModuleGateRemoved(Module* m, const u32 removed_gate);

        /**
         * Handler to be called after an input port of a module has been renamed. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param m - The module with the renamed input port
         * @param net - The net that is connected to the renamed input port
         */
        void handleModuleInputPortNameChanged(Module* m, const u32 net);

        /**
         * Handler to be called after an output port of a module has been renamed. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param m - The module with the renamed output port
         * @param net - The net that is connected to the renamed output port
         */
        void handleModuleOutputPortNameChanged(Module* m, const u32 net);

        /**
         * Handler to be called after a gate has been renamed. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param g - The renamed gate
         */
        void handleGateNameChanged(Gate* g) const;

        /**
         * Handler to be called after a new net has been created. <br>
         * Currently no logic is done here.
         *
         * @param n - The new net
         */
        void handleNetCreated(Net* n) const;

        /**
         * Handler to be called after a net has been removed. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param n - The removed net
         */
        void handleNetRemoved(Net* n) const;
        /**
         * Handler to be called after a net has been renamed. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param n - The renamed net
         */
        void handleNetNameChanged(Net* n) const;

        /**
         * Handler to be called after a source has been added to a net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param n - The net with the new source
         * @param src_gate_id - The id of the gate that contains the source pin
         */
        void handleNetSourceAdded(Net* n, const u32 src_gate_id) const;

        /**
         * Handler to be called after a source has been removed from a net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param n - The net with the removed source
         * @param src_gate_id - The id of the gate that contains the source pin
         */
        void handleNetSourceRemoved(Net* n, const u32 src_gate_id) const;

        /**
         * Handler to be called after a destination has been added to a net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param n - The net with the new destination
         * @param dst_gate_id - The id of the gate that contains the destination pin
         */
        void handleNetDestinationAdded(Net* n, const u32 dst_gate_id) const;

        /**
         * Handler to be called after a destination has been removed from a net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param n - The net with the removed destination
         * @param dst_gate_id - The id of the gate that contains the destination pin
         */
        void handleNetDestinationRemoved(Net* n, const u32 dst_gate_id) const;

        /**
         * Handler to be called after a net has been marked as a global input net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param mNetId - The id of the net that has been marked as a global input
         */
        void handleMarkedGlobalInput(u32 mNetId);

        /**
         * Handler to be called after a net has been marked as a global output net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param mNetId - The id of the net that has been marked as a global output
         */
        void handleMarkedGlobalOutput(u32 mNetId);

        /**
         * Handler to be called after a net is no longer a global input net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param mNetId - The id of the net that is no longer a global input
         */
        void handleUnmarkedGlobalInput(u32 mNetId);

        /**
         * Handler to be called after a net is no longer a global output net. <br>
         * Used to apply the changes in the affected contexts.
         *
         * @param mNetId - The id of the net that is no longer a global output
         */
        void handleUnmarkedGlobalOutput(u32 mNetId);

        /**
         * Assigns new ID to context if this id is not in use
         *
         * @param ctx the context
         * @param ctxId user provided ID
         */
        void setContextId(GraphContext* ctx, u32 ctxId);

        /**
         * Creates an instance of the StandardGraphLayouter used for a certain GraphContext.
         *
         * Note: The calling party must assure that the layouter is delegated to the context using GraphContext::setLayouter
         *
         * @param context - The context the layouter works on
         * @returns a pointer to the new GraphLayouter
         */
        GraphLayouter* getDefaultLayouter(GraphContext* const context) const;

        /**
         * Creates an instance of ModuleShader used for a certain GraphContext.
         *
         * Note: The calling party must assure that the shader is delegated to the context using GraphContext::setShader
         *
         * @param context - The context the shader works on
         * @returns a pointer to the new GraphShader
         */
        GraphShader* getDefaultShader(GraphContext* const context) const;

        /**
         * Gets the table model for the contexts.
         *
         * @returns the context table model
         */
        ContextTableModel* getContextTableModel() const;

        /**
         * Deletes all contexts.
         */
        void clear();
        void handleSaveTriggered();
        void restoreFromFile();

        QString nextDefaultName() const { return QString("view %1").arg(mMaxContextId+1);}
    Q_SIGNALS:
        /**
         * Q_SIGNAL that notifies about the creation of a new context by the context manager.
         *
         * @param context - The created context
         */
        void contextCreated(GraphContext* context);

        /**
         * Q_SIGNAL that notifies about the renaming of a context by the context manager.
         *
         * @param context - The renamed context
         */
        void contextRenamed(GraphContext* context);

        /**
         * Q_SIGNAL that notifies that a context is about to be deleted. <br>
         * This signal is emitted before the context is removed from the manager and deleted.
         *
         * @param context - The context that is about to be deleted
         */
        void deletingContext(GraphContext* context);

    private:
//        QVector<GraphContext*> mGraphContexts;

        ContextTableModel* mContextTableModel;
        u32 mMaxContextId;
        void dump(const QString& title, u32 mid, u32 xid) const;
        SettingsItemCheckbox* mSettingDebugGrid;
        SettingsItemCheckbox* mSettingNetLayout;
        SettingsItemCheckbox* mSettingParseLayout;
        SettingsItemCheckbox* mSettingLayoutBoxes;
    };
}

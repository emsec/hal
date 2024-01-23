// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <QSet>

namespace hal
{
    class GraphWidget;

    /**
     * @ingroup graph-contexts
     * @brief Logical container for modules, gates, and nets.
     *
     * The GraphContext is a container that holds all elements (Module%s, Gate%s, Net%s) that should be drawn in the
     * scene. It uses a GraphLayouter to layout its elements and creates a scene with the corresponding GraphicsItem%s.
     * Afterwards this scene can be shown in the GraphWidget's GraphGraphicsView. <br>
     * Moreover the context may be changed (e.g. add/remove Gate%s or Module%s). In this case the scene will be adapted
     * as well. <br>
     * This class notifies the GraphWidget parent about
     * certain events.
     */
    class GraphContext : public QObject
    {
        friend class GraphContextManager;
        friend class LayoutLockerManager;
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param id_ - The unique id of the context.
         * @param name - The name of the context
         * @param parent - The parent QObject
         */
        explicit GraphContext(u32 id_, const QString& name, QObject* parent = nullptr);

        /**
         * Destructor.
         * Used to notify all subscribers about the deletion.
         */
        ~GraphContext();

        /**
         * Mark the beginning of a block of changes that are done successively. Used to prevent the scene from updating
         * after every single step. The scene will be only updated after endChange is called. <br>
         * These 'changes-blocks' may be nested. In this case only the outer block will be considered.
         */
        void beginChange();

        /**
         * Mark the ending of a block of changes that are done successively. Used to prevent the scene from updating
         * after every single step.
         */
        void endChange();

        /**
         * Add Gate%s and/or Module%s to the context. The scene will be updated afterwards
         * (if not in a beginChanges-endChanges-block). Moreover a placement hint may be passed that can be used by the
         * GraphLayouter.
         *
         * @param modules - A set of ids of the Module%s to be added
         * @param gates - A set of ids of the Gate%s to be added
         * @param placement - A placement hint (leave empty for no hint)
         */
        void add(const QSet<u32>& modules, const QSet<u32>& gates, PlacementHint placement = PlacementHint());

        /**
         * Remove Gate%s and/or Module%s from the context. The scene will be updated afterwards
         * (if no in a beginChanges-endChanges-block).
         *
         * @param modules - A set of ids of the modules to be removed
         * @param gates - A set of ids of the gates to be removed
         */
        void remove(const QSet<u32>& modules, const QSet<u32>& gates);

        /**
         * Clear the context. All Gate%s and Module%s as well as their placement hints will be removed.
         */
        void clear();

        /**
         * Fold the parent module of a specific gate. The specified gate as well as all other Gate%s and Submodule%s of the parent
         * module are removed from the context and replaced by the module itself.
         *
         * @param gateId - The id of the gate
         */
        bool isGateUnfolded(u32 gateId) const;

        /**
         * Checks whether a module is unfolded or not.
         *
         * @param moduleId - The id of the module
         */
        bool isModuleUnfolded(const u32 moduleId) const;

        /**
         * Unfold a specific module. The specified module is removed from the context and replaced by its Gate%s and
         * submodules.
         *
         * @param id - The id of the module to unfold
         */
        void unfoldModule(const u32 id, const PlacementHint& plc);

        /**
         * Check if the context is empty i.e. does not contain Module%s or Gate%s.
         *
         * @returns <b>true</b> if the context is empty.
         */
        bool empty() const;

        /**
         * Checks whether the context will be empty after pending changes are applied.
         * @return <b>true</b> if the context is empty.
         */
        bool willBeEmptied() const;

        /**
         * Checks if the context represents the content of the given module i.e. after double-clicking the module item.
         *
         * @param id - The id of the module
         * @returns <b>true</b> if the context shows the content of the module
         */
        bool isShowingModule(const u32 id) const;

        /**
         * Checks if the context represents the content of the given module i.e. after double-clicking the module item. <br>
         * In some cases it is necessary that the check is performed on a previous state e.g. if a gate was newly assigned to the
         * netlist and the affected GraphContexts should be discovered now. Since the comparison is done on the current
         * netlist but the context does not contain the gate yet, this function will falsely return <b>false</b> even
         * on the right GraphContexts. Therefore the new gate must be manually removed from the comparison by passing
         * it in the minus_gates list. <br>
         * Accordingly, it is also possible to add Gate%s to the comparison (e.g. after removing them from the module)
         * as well as adding and removing Module%s (i.e. submodules).
         *
         * @param id - The id of the module
         * @param minus_modules - The ids of the Module%s that are removed for comparison
         * @param minus_gates - The ids of the Gate%s that are removed for comparison
         * @param plus_modules - The ids of the Module%s that are added for comparison
         * @param plus_gates - The ids of the Gate%s that are added for comparison
         * @returns <b>true</b> if the context Show%s the content of the module.
         */
        bool isShowingModule(const u32 id, const QSet<u32>& minus_modules, const QSet<u32>& minus_gates, const QSet<u32>& plus_modules, const QSet<u32>& plus_gates) const;

        /**
         * Checks wether the context shows an module exclusively or not.
         */
        bool isShowingModuleExclusively();

        /**
         * Recursively get all gates and submodules of module visible in graph context
         *
         * @param moduleId - The id of the module
         * @param gates - Gates of module
         * @param modules - Submodules of module
         */
        void getModuleChildrenRecursively(const u32 moduleId, QSet<u32>* gates, QSet<u32>* modules) const;

        /**
         * Convenience function to allow calls to GraphWidget::storeViewport via context
         */
        void storeViewport();

        /**
         * Called by layouter to signal progress
         * @param percent
         */
        void layoutProgress(int percent) const;

        /**
         * Checks whether a modification affects the context and schedules an scene update if necessary.
         *
         * @param id - The id of the modified module
         * @param moduleId - The id of the module which was added/removed
         * @param gateId - The id of the gate which was added/removed
         */
        void testIfAffected(const u32 id, const u32* moduleId, const u32* gateId);

        /**
         * Given a net, this function checks if any of the Net's source Gate%s appear in the context.
         *
         * @param mNetId - The id of the net
         * @returns <b>true</b> if a source gate of the net is shown
         */
        bool isShowingNetSource(const u32 mNetId) const;

        /**
         * Given a net, this functions checks if any of the Net's destination gates appear in the context.
         *
         * @param mNetId - The id of the net
         * @returns <b>true</b> if a destination gate of the net is shown
         */
        bool isShowingNetDestination(const u32 mNetId) const;

        /**
         * Checks whether there is only the folded top_module in the context
         * which makes other time consumptive tests unnecessary
         * @return <b>true</b> If here is only the folded top_module in the context
         */
        bool isShowingFoldedTopModule() const;

        /**
         * Given a net, this function returns the first visible source node.
         *
         * @param n - Pointer to net
         * @returns <b>true</b> The first visibible source node, might be Node::None
         */
        Node getNetSource(const Net* n) const;

        /**
         * Given a net, this function returns the first visible destination node.
         *
         * @param n - Pointer to net
         * @returns <b>true</b> The first visibible destination node, might be Node::None
         */
        Node getNetDestination(const Net* n) const;

        /**
         * Get the ids of the Module%s of the context.
         *
         * @returns a set of Module ids.
         */
        const QSet<u32>& modules() const;

        /**
         * Get the ids of the Gates of the context.
         *
         * @returns a set of gate ids.
         */
        const QSet<u32>& gates() const;

        /**
         * Get the ids of the Net%s of the context.
         *
         * @returns a set of net ids.
         */
        const QSet<u32>& nets() const;

        /**
         * Get the scene the context works on.
         *
         * @returns the scene.
         */
        GraphicsScene* scene();

        /**
         * Get the name of the context.
         *
         * @returns the context's name.
         */
        QString name() const;

        /**
         * Get the name of the context with dirty state.
         *
         * @returns the context's name with an asterisk if the dirty is set to true.
         */
        QString getNameWithDirtyState() const;

        /**
         * Get the id of the context.
         *
         * @return the context's id.
         */
        u32 id() const;

        /**
         * Set the GraphLayouter this context should use.
         *
         * @param layouter - The GraphLayouter to use
         */
        void setLayouter(GraphLayouter* layouter);

        /**
         * Set the GraphShader that will be used.
         *
         * @param shader - The GraphShader
         */
        void setShader(GraphShader* shader);

        /**
         * Get the used GraphLayouter.
         * Returns a <i>nullptr</i> if no layouter was configured yet.
         *
         * @returns the used GraphLayouter
         */
        GraphLayouter* getLayouter() const
        {
            return mLayouter;
        }

        /**
		 * Move node to antother grid location
		 */
        void moveNodeAction(const QPoint& from, const QPoint& to);

        /**
         * Returns whether the scene is in an updating process (i.e. layouter process) or not.
         *
         * @returns <b>true</b> while the scene updates.
         */
        bool sceneUpdateInProgress() const;

        /**
         * Notifies the context that a scene update is necessary. The scene will be updated immediately or as soon as
         * the exterior beginChanges-endChanges-block is left.
         */
        void scheduleSceneUpdate();

        /**
         * Get a Node that contains a specific Gate. <br>
         * If the context contains the Gate, the node of this gate is returned. <br>
         * If the context only contains a parent module of the gate (or the parent's parent and so on...), the node of
         * this Module is returned. <br>
         * Else an empty node is returned.
         *
         * @param id - The Gate's id
         * @returns the Node that contains the gate
         */
        Node nodeForGate(const u32 id) const;

        /**
         * Returns the timestamp of this context. The timestamp is generated in the constructor.
         *
         * @returns the timestamp of the context.
         */
        QDateTime getTimestamp() const;

        /**
         * Writes the context (its modules, gates, nets) to a given json object.
         *
         * @param json - The object to write to.
         */
        void writeToFile(QJsonObject& json);

        /**
         * Reads a context from a given json object.
         *
         * @param json - The object to read from.
         * @return true if reading was successful, false otherwise
         */
        bool readFromFile(const QJsonObject& json);

        /**
         * Sets the dirty state.
         *
         * @param dty - The value to set.
         */
        void setDirty(bool dty);

        /**
         * Get the dirty state.
         *
         * @return The dirty state.
         */
        bool isDirty() const
        {
            return mDirty;
        }

        /**
         * Set the special update state.
         */
        void setSpecialUpdate(bool state);

        /**
         * Get the special update state.
         *
         * @return The special update state.
         */
        bool getSpecialUpdate() const
        {
            return mSpecialUpdate;
        }

        /**
         * Set pointer to parent graph widget
         * @param[in] gw parent of class GraphWidget
         */
        void setParentWidget(GraphWidget* gw)
        {
            mParentWidget = gw;
        }

        /**
         * Get the exclusive module id.
         *
         * @return The exclusive module id.
         */
        u32 getExclusiveModuleId()
        {
            return mExclusiveModuleId;
        }

        /**
         * Sets the exclusive module id.
         */
        void setExclusiveModuleId(u32 id, bool emitSignal = true);

        /**
         * Checks whether context still shows module with id mExclusiveModuleId exclusively. If not, mExclusiveModuleId is set to 0.
         */
        void exclusiveModuleCheck();

        /**
         * Open overlay and show nodes
         * @param nd The node for which comments should be shown
         */
        void showComments(const Node& nd);

        /**
         * Update set of nets so that all connections to gates or modules within view are shown
         */
        void updateNets();

        /**
         * Make sure these nodes gets removed from context
         */
        void setScheduleRemove(const QSet<u32>& mods, const QSet<u32>& gats);

        /**
         * Check whether node is scheduled for removal
         */
        bool isScheduledRemove(const Node& nd);

    Q_SIGNALS:
        void dataChanged();
        void exclusiveModuleLost(u32 old_id);

    public Q_SLOTS:
        void abortLayout();

    private Q_SLOTS:
        void handleLayouterFinished();
        void handleStyleChanged(int istyle);
        void handleExclusiveModuleLost(u32 old_id);
        void handleModuleNameChanged(Module* m);

    private:
        void evaluateChanges();
        void update();
        void applyChanges();
        void requireSceneUpdate();
        void startSceneUpdate();
        bool testIfAffectedInternal(const u32 id, const u32* moduleId, const u32* gateId);
        void removeModuleContents(const u32 moduleId);

        u32 mId;
        QString mName;
        GraphWidget* mParentWidget;

        bool mDirty;

        GraphLayouter* mLayouter;
        GraphShader* mShader;

        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        QSet<u32> mAddedModules;
        QSet<u32> mAddedGates;

        QList<PlacementEntry> mPlacementList[4];

        QSet<u32> mRemovedModules;
        QSet<u32> mRemovedGates;

        u32 mUserUpdateCount;

        bool mUnappliedChanges;
        bool mSceneUpdateRequired;
        bool mSceneUpdateInProgress;

        QDateTime mTimestamp;

        bool mSpecialUpdate;

        QSet<u32> mScheduleRemoveModules;
        QSet<u32> mScheduleRemoveGates;

        u32 mExclusiveModuleId;
    };
}    // namespace hal

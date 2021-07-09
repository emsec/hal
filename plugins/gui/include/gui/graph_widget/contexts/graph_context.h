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

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QObject>
#include <QSet>
#include <QDateTime>
#include <QJsonObject>

namespace hal
{
    class GraphContextSubscriber;

    /**
     * @ingroup graph-contexts
     * @brief Logical container for modules, gates, and nets.
     *
     * The GraphContext is a container that holds all elements (Module%s, Gate%s, Net%s) that should be drawn in the
     * scene. It uses a GraphLayouter to layout its elements and creates a scene with the corresponding GraphicsItem%s.
     * Afterwards this scene can be shown in the GraphWidget's GraphGraphicsView. <br>
     * Moreover the context may be changed (e.g. add/remove Gate%s or Module%s). In this case the scene will be adapted
     * as well. <br>
     * This class implements an observer pattern to notify subscribers (currently only GraphWidget objects) about
     * certain events.
     */
    class GraphContext : public QObject
    {
        friend class GraphContextManager;
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
         * Register a subscriber that will notified about certain changes (see GraphContextSubscriber).
         *
         * @param subscriber - The GraphContextSubscriber to subscribe
         */
        void subscribe(GraphContextSubscriber* const subscriber);

        /**
         * Remove a subscriber.
         *
         * @param subscriber - The GraphContextSubscriber to unsubscribe
         */
        void unsubscribe(GraphContextSubscriber* const subscriber);

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
         * Folds a given module with a given placement hint.
         *
         * @param moduleId - The module to fold.
         * @param plc - The placement hint.
         * @return True on success, False otherwise.
         */
        bool foldModuleAction(u32 moduleId, const PlacementHint& plc);

        /**
         * Unfold a specific module. The specified module is removed from the context and replaced by its Gate%s and
         * submodules.
         *
         * @param id - The id of the module to unfold
         */
        void unfoldModule(const u32 id);

        /**
         * Check if the context is empty i.e. does not contain Module%s or Gate%s.
         *
         * @returns <b>true</b> if the context is empty.
         */
        bool empty() const;

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
         * Given a net, this function returns the first visible source node.
         *
         * @param n - Pointer to net
         * @returns <b>true</b> The first visibible source node, might be Node::None
         */
        Node getNetSource(const Net *n) const;

        /**
         * Given a net, this function returns the first visible destination node.
         *
         * @param n - Pointer to net
         * @returns <b>true</b> The first visibible destination node, might be Node::None
         */
        Node getNetDestination(const Net *n) const;

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
        GraphLayouter* getLayouter() const { return mLayouter; }

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
         */
        void readFromFile(const QJsonObject& json);

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
        bool isDirty() const {return mDirty; }

    Q_SIGNALS:
        void dataChanged();

    private Q_SLOTS:
        void handleLayouterUpdate(const int percent);
        void handleLayouterUpdate(const QString& message);
        void handleLayouterFinished();
        void handleStyleChanged(int istyle);

    private:
        void evaluateChanges();
        void update();
        void applyChanges();
        void startSceneUpdate();
        bool testIfAffectedInternal(const u32 id, const u32* moduleId, const u32* gateId);

        QList<GraphContextSubscriber*> mSubscribers;

        u32 mId;
        QString mName;
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
    };
}

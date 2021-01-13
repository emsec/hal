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

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QObject>
#include <QSet>
#include <QDateTime>

namespace hal
{
    class GraphContextSubscriber;

    /**
     * The GraphContext is a container that holds all elements (modules, gates, nets) that should be drawn in the
     * scene. It uses a GraphLayouter to layout its elements and creates a scene with the corresponding graphic items.
     * Afterwards this scene can be shown in a GraphWidgets GraphGraphicsView. <br>
     * Moreover the context may be changed (e.g. add/remove gates or modules). In this case the scene will be adapted
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
         * @param name - The name of the context
         * @param parent - The parent QObject
         */
        explicit GraphContext(const QString& name, QObject* parent = nullptr);

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
         * Add gates and/or modules to the context. The scene will be updated afterwards
         * (if no in a beginChanges-endChanges-block). Moreover a placement hint may be passed that can be used by the
         * GraphLayouter.
         *
         * @param modules - A set of ids of the modules to be added
         * @param gates - A set of ids of the gates to be added
         * @param placement - A placement hint (leave empty for no hint)
         */
        void add(const QSet<u32>& modules, const QSet<u32>& gates, PlacementHint placement = PlacementHint());

        /**
         * Remove gates and/or modules from the context. The scene will be updated afterwards
         * (if no in a beginChanges-endChanges-block).
         *
         * @param modules - A set of ids of the modules to be removed
         * @param gates - A set of ids of the gates to be removed
         */
        void remove(const QSet<u32>& modules, const QSet<u32>& gates);

        /**
         * Clear the context. All gates and modules as well as their placement hints will be removed.
         */
        void clear();

        /**
         * Fold the parent module of a specific gate. The specified gate as well as all other gates and submodules of the parent
         * module are removed from the context and replaced by the module itself.
         *
         * @param id - The id of the gate
         */
        void foldModuleOfGate(const u32 id);

        /**
         * Unfold a specific module. The specified module is removed from the context and replaced by its gates and
         * submodules.
         *
         * @param id - The id of the module to unfold
         */
        void unfoldModule(const u32 id);

        /**
         * Check if the context is empty i.e. does not contain modules or gates.
         *
         * @returns <b>true</b> if the context is empty.
         */
        bool empty() const;

        /**
         * TODO: Documentation
         *
         * @param id
         * @return
         */
        bool isShowingModule(const u32 id) const;
        /**
         * TODO: Documentation
         *
         * @param id
         * @param minus_modules
         * @param minus_gates
         * @param plus_modules
         * @param plus_gates
         * @return
         */
        bool isShowingModule(const u32 id, const QSet<u32>& minus_modules, const QSet<u32>& minus_gates, const QSet<u32>& plus_modules, const QSet<u32>& plus_gates) const;

        /**
         * TODO: Documentation
         *
         * @param mNetId
         * @return
         */
        bool isShowingNetSource(const u32 mNetId) const;

        /**
         * TODO: Documentation
         *
         * @param mNetId
         * @return
         */
        bool isShowingNetDestination(const u32 mNetId) const;

        /**
         * Get the ids of the modules of the context.
         *
         * @returns a set of module ids.
         */
        const QSet<u32>& modules() const;

        /**
         * Get the ids of the gates of the context.
         *
         * @returns a set of gate ids.
         */
        const QSet<u32>& gates() const;

        /**
         * Get the ids of the nets of the context.
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
         * @returns the contexts name.
         */
        QString name() const;

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
        const GraphLayouter* getLayouter() const { return mLayouter; }

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
         * Get a Node that contains a specific gate. <br>
         * If the context contains the gate, the node of this gate is returned. <br>
         * If the context only contains a parent module of the gate (or the parents parent and so on...), the node of
         * this module is returned. <br>
         * Else an empty node is returned.
         *
         * @param id - The gates id
         * @returns the Node that contains the gate
         */
        Node nodeForGate(const u32 id) const;

        /**
         * TODO: remove?
         * Get the used GraphLayouter. Originally used for debug purposes.
         * \deprecated Please use getLayouter() instead.
         *
         * @returns the used GraphLayouter
         */
        GraphLayouter* debugGetLayouter() const;

        /**
         * Returns the timestamp of this context. The timestamp is generated in the constructor.
         *
         * @returns the timestamp of the context.
         */
        QDateTime getTimestamp() const;

    private Q_SLOTS:
        void handleLayouterUpdate(const int percent);
        void handleLayouterUpdate(const QString& message);
        void handleLayouterFinished();

    private:
        void evaluateChanges();
        void update();
        void applyChanges();
        void startSceneUpdate();

        QList<GraphContextSubscriber*> mSubscribers;

        QString mName;

        GraphLayouter* mLayouter;
        GraphShader* mShader;

        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        QSet<u32> mAddedModules;
        QSet<u32> mAddedGates;

        QMultiMap<PlacementHint, u32> mModuleHints;
        QMultiMap<PlacementHint, u32> mGateHints;

        QSet<u32> mRemovedModules;
        QSet<u32> mRemovedGates;

        u32 mUserUpdateCount;

        bool mUnappliedChanges;
        bool mSceneUpdateRequired;
        bool mSceneUpdateInProgress;

        QDateTime mTimestamp;
    };
}

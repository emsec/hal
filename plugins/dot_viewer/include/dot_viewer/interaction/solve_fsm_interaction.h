// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS").
// All Rights reserved.
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

#include "QGVCore/QGVScene.h"
#include "hal_core/defines.h"
#include <QHash>
#include <QMap>

namespace hal {
    class Net;
    
    /**
     * Adds the FSM solver actions to the context menu of the nodes and edges of a displayed DOT graph.
     */
    class SolveFsmInteraction : public QGVInteraction {
        Q_OBJECT
    public:
        /**
         * Construct the interaction for the given scene.
         *
         * @param[in] parent - The scene that holds the displayed graph.
         */
        SolveFsmInteraction(QGVScene* parent);

        ~SolveFsmInteraction();

        /**
         * Attach the interaction to a node of the graph, which makes the state it represents selectable.
         *
         * @param[in] node - The node to attach to.
         */
        void registerNode(QGVNode* node) override;

        /**
         * Attach the interaction to an edge of the graph, which makes the transition it represents selectable.
         *
         * @param[in] edge - The edge to attach to.
         */
        void registerEdge(QGVEdge* edge) override;

    private Q_SLOTS:
        /**
         * Highlight the graph elements that correspond to the netlist elements selected in HAL.
         *
         * @param[in] sender - The object that changed the selection.
         */
        void handleHALSelectionChanged(void* sender);

        /**
         * Select the netlist elements that correspond to the graph elements selected in the scene.
         */
        void handleQGVSelectionChanged();

        /**
         * Show the context menu of the given edge, which offers the transition of the FSM it represents.
         *
         * @param[in] edge - The edge that was right-clicked.
         */
        void handleEdgeContextMenu(QGVEdge* edge);

        /**
         * Keep track of the area covered by the graph so that the highlight can be redrawn.
         *
         * @param[in] changedArea - The regions of the scene that changed.
         */
        void handleSceneChanged(const QList<QRectF>& changedArea);

    private:
        /** A map from each net to the edges that represent a transition depending on it. */
        QMultiHash<u32, QGVEdge*> mNetHash;

        /** The scene that holds the displayed graph. */
        QGVScene* mScene;

        /** The area of the scene that the graph covers. */
        QRectF mItemArea;

        /**
         * Get the input values that cause the transition represented by the given edge.
         *
         * @param[in] edge - The edge representing the transition.
         * @returns A map from each input net to the value it has to assume.
         */
        QMap<Net*,int> transitionValues(QGVEdge* edge) const;

        /**
         * Remove the highlight from all edges of the graph.
         */
        void resetEdgeHighlight();
    };

    /**
     * Create the FSM solver interaction for the given scene.
     *
     * @param[in] parent - The scene that holds the displayed graph.
     * @returns The created interaction.
     */
    QGVInteraction* constructSolveFsmInteraction(QGVScene* parent);

    /**
     * Registers the FSM solver interaction with the DOT viewer on program startup.
     */
    class SolveFsmInteractionRegistration {
        /** The single instance whose construction performs the registration. */
        static SolveFsmInteractionRegistration sRegistration;

    public:
        /**
         * Register the FSM solver interaction with the DOT viewer.
         */
        SolveFsmInteractionRegistration();
    };

}


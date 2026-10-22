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

namespace hal {
    /**
     * Adds the clock tree extractor actions to the context menu of the nodes and edges of a displayed DOT graph.
     */
    class ClockTreeExtractorInteraction : public QGVInteraction {
        Q_OBJECT
    public:
        /**
         * Construct the interaction for the given scene.
         *
         * @param[in] parent - The scene that holds the displayed graph.
         */
        ClockTreeExtractorInteraction(QGVScene* parent);

        /**
         * Attach the interaction to a node of the graph, which makes the gate or net it represents selectable.
         *
         * @param[in] node - The node to attach to.
         */
        void registerNode(QGVNode* node) override;

        /**
         * Attach the interaction to an edge of the graph, which makes the connection it represents selectable.
         *
         * @param[in] scene - The edge to attach to.
         */
        void registerEdge(QGVEdge* scene) override;

    private Q_SLOTS:
        /**
         * Highlight the graph nodes that correspond to the netlist elements selected in HAL.
         *
         * @param[in] sender - The object that changed the selection.
         */
        void handleHALSelectionChanged(void* sender);

        /**
         * Select the netlist elements that correspond to the graph nodes selected in the scene.
         */
        void handleQGVSelectionChanged();

    private:
        /** A map from each gate to the graph node that represents it. */
        QHash<u32, QGVNode*> mGateHash;

        /** A map from each graph node to the gate that it represents. */
        QHash<QGVNode*, u32> mNodeHash;

        /** A map from each net to the graph node that represents it. */
        QHash<u32, QGVNode*> mNetHash;

        /** A map from each graph node to the global input net that it represents. */
        QHash<QGVNode*, u32> mGlobalInputHash;

        /** The scene that holds the displayed graph. */
        QGVScene* mScene;
    };

    /**
     * Create the clock tree extractor interaction for the given scene.
     *
     * @param[in] parent - The scene that holds the displayed graph.
     * @returns The created interaction.
     */
    QGVInteraction* constructClockTreeExtractorInteraction(QGVScene* parent);
    /**
     * Registers the clock tree extractor interaction with the DOT viewer on program startup.
     */
    class ClockTreeExtractorInteractionRegistration {
        /** The single instance whose construction performs the registration. */
        static ClockTreeExtractorInteractionRegistration sRegistration;

    public:
        /**
         * Register the clock tree extractor interaction with the DOT viewer.
         */
        ClockTreeExtractorInteractionRegistration();
    };

}


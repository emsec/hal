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

#include <QVector>
#include <QPoint>
#include <QHash>
#include <QSet>
#include "gui/gui_def.h"
#include "hal_core/netlist/gate.h"

uint qHash(const QPoint& p);

namespace hal {
    class GraphicsNode;

    /**
     * @ingroup graph-layouter
     * @brief The NodeBox class represents a node placed at a grid position within a hal view.
     *
     * The constructur of NodeBox creates the box item derived from GraphicsNode/QGraphicsItem.
     * Constructor should be called by layouter since scene position needs to be assigned to
     * scene. Each hal view has its own list of NodeBox'es which should be stored in class
     * NodeBoxes.
     */
    class NodeBox
    {
        Node mNode;
        GraphicsNode* mItem;

        int mX;
        int mY;

        qreal mInputPadding;
        qreal mOutputPadding;

        static const int sMinimumGateIoPadding  = 60;

    public:
        /**
         * @brief NodeBox constructor creates graphic item
         * @param n node of type gate or module
         * @param px x grid position where box is placed
         * @param py y grid position where box is placed
         */
        NodeBox(const Node& n, int px, int py);

        /**
         * @brief getNode getter for node information
         * @return the node represented by NodeBox
         */
        Node getNode() const { return mNode; }

        /**
         * @brief type getter for type information
         * @return the type should be either Node::Module or Node::Gate
         */
        Node::NodeType type() const { return mNode.type(); }

        /**
         * @brief id getter for ID information
         * @return the id as unsigned integer
         */
        u32 id() const { return mNode.id(); }

        /**
         * @brief x getter for X-grid position
         * @return x-grid position
         */
        int x() const { return mX; }

        /**
         * @brief y getter for Y-grid position
         * @return y-grid position
         */
        int y() const { return mY; }

        /**
         * @brief setItem
         * @param item_
         */
        void setItem(GraphicsNode* item_) { mItem = item_; }

        /**
         * @brief setItemPosition place item on scene at real position
         * @param xpos x-scene position
         * @param ypos y-scene position
         */
        void setItemPosition(qreal xpos, qreal ypos);

        /**
         * @brief item getter for graphics item object
         * @return pointer to item
         */
        GraphicsNode* item() const { return mItem; }

        /**
         * @brief inputPadding distance from top edge to first input net
         * @return input padding as real value
         */
        qreal inputPadding() const { return mInputPadding; }

        /**
         * @brief outputPadding distance from top edge to first output net
         * @return output padding as real value
         */
        qreal outputPadding() const { return mOutputPadding; }
    };

    /**
     * @brief The NodeBoxes class owns all NodeBox'es from hal view.
     *
     * @ingroup graph-layouter
     *
     * Several index members (QHash) allow fast access by grid position, node id,
     * mouse click in graphics, or net endpoint.
     * Pointer to new NodeBox'es should only be added using the addBox() method
     * to update index correctly.
     */
    class NodeBoxes : public QVector<NodeBox*>
    {
    public:
        /**
         * @brief addBox call NodeBox constructor and store pointer in vector
         * @param nd Node to be created and added
         * @param px grid x-position
         * @param py grid y-position
         */
        void addBox(const Node& nd, int px, int py);

        /**
         * @brief clearBoxes delete all NodeBox'es and clear vector.
         */
        void clearBoxes();

        /**
         * @brief boxForGate find NodeBox by Gate pointer.
         * @param g pointer to netlist Gate
         * @return pointer to NodeBox or nullptr if none found
         *
         * A NodeBox is returned if the box either represents the given gate or
         * if the gate is part of a module rendered as box. The method ist mostly
         * used to find boxes by net endpoints.
         */
        NodeBox* boxForGate(const Gate* g) const { return mGateHash.value(g); }

        /**
         * @brief boxForPoint find NodeBox by grid position
         * @param p integer grid position
         * @return pointer to NodeBox or nullptr if none found
         */
        NodeBox* boxForPoint(QPoint p) const { return mPointHash.value(p); }

        /**
         * @brief boxForNode find NodeBox by node
         * @param n the node
         * @return pointer to NodeBox or nullptr if none found
         *
         * Used to search gates or modules by ID.
         */
        NodeBox* boxForNode(Node& n) const { return mNodeHash.value(n); }

        /**
         * @brief boxForItem find NodeBox by graphics item
         * @param item pointer to graphics item
         * @return pointer to NodeBox or nullptr if none found
         *
         * Mostly used to find NodeBox selected in QGraphicsView engine, e.g. by mouse position
         */
        NodeBox* boxForItem(GraphicsNode* item) const { return mItemHash.value(item); }

        QSet<u32> filterNotInView(const QSet<u32>& gats) const;
    private:
        QHash<const Gate*,NodeBox*>   mGateHash;
        QHash<QPoint,NodeBox*>        mPointHash;
        QHash<Node,NodeBox*>          mNodeHash;
        QHash<GraphicsNode*,NodeBox*> mItemHash;
    };
}

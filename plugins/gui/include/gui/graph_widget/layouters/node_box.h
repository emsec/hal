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

#include <QVector>
#include <QPoint>
#include <QHash>
#include "gui/gui_def.h"
#include "hal_core/netlist/gate.h"

uint qHash(const QPoint& p);

namespace hal {
    uint qHash(const Node &n);

    class GraphicsNode;

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
        NodeBox(const Node& n, int px, int py);
        Node getNode() const { return mNode; }
        Node::NodeType type() const { return mNode.type(); }
        u32 id() const { return mNode.id(); }
        int x() const { return mX; }
        int y() const { return mY; }
        void setItem(GraphicsNode* item_) { mItem = item_; }
        void setItemPosition(qreal xpos, qreal ypos);
        GraphicsNode* item() const { return mItem; }
        qreal inputPadding() const { return mInputPadding; }
        qreal outputPadding() const { return mOutputPadding; }
    };

    class NodeBoxes : public QVector<NodeBox*>
    {
    public:
        void addBox(const Node& nd, int px, int py);
        void clearBoxes();
        NodeBox* boxForGate(Gate* g) const { return mGateHash.value(g); }
        NodeBox* boxForPoint(QPoint p) const { return mPointHash.value(p); }
        NodeBox* boxForNode(Node& n) const { return mNodeHash.value(n); }
        NodeBox* boxForItem(GraphicsNode* item) const { return mItemHash.value(item); }
    private:
        QHash<Gate*,NodeBox*>         mGateHash;
        QHash<QPoint,NodeBox*>        mPointHash;
        QHash<Node,NodeBox*>          mNodeHash;
        QHash<GraphicsNode*,NodeBox*> mItemHash;
    };
}

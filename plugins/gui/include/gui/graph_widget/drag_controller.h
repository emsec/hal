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
#include <QObject>
#include <QSet>
#include <QPoint>
#include <QHash>
#include "gui/gui_def.h"
#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"

namespace hal {

    class GraphWidget;
    class GraphicsNode;
    class NodeBox;
    class NodeDragShadow;
    class GraphicsScene;

    class DragController : public QObject
    {
        Q_OBJECT
        GraphWidget* mGraphWidget;
        bool mDropAllowed;
        bool mWantSwap;
        QPoint mMousedownPosition;
        NodeBox* mDragNodeBox;
        QPoint mCurrentGridpos;
        QSet<NodeBox*> mAdditionalBoxes;
        QHash<const NodeBox*,NodeDragShadow*> mShadows;
        GraphicsScene* mShadowScene;

        void setSwapIntent(bool wantSwap);
        void addShadow(const NodeBox* nb);
    public:
        DragController(GraphWidget* gw, QObject* parent = nullptr);

        void clear();

        void set(GraphicsNode* drgItem, const QPoint& eventPos);

        /**
         * Starts the dragging of a gate or module to show its shadow meanwhile.
         *
         * @param wantSwap - True if keyboard swap modifier has been pressed, false otherwise
         */
        void enterDrag(bool wantSwap);

        /**
         * Moves the shadow that appears while dragging a gate or module.
         *
         * @param eventPos - The mouse position in scene coordinates while dragging
         * @param wantSwap - True if keyboard swap modifier has been pressed, false otherwise
         * @param gridPos - The new grid position of the primary node
         */
        void move(const QPoint& eventPos, bool wantSwap, const QPoint& gridPos);

        /**
         * Remove all painted shadows from graphics scene
         */
        void clearShadows(GraphicsScene* sc);

        bool hasDragged(const QPoint& eventPos);
        bool isDropAllowed() const;
        GridPlacement* finalGridPlacement() const;
        NodeDragShadow::DragCue dragCue() const;
    };
}

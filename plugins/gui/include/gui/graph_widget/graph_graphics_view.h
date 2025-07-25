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

#include "hal_core/defines.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "gui/gui_globals.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/module_dialog/gate_select_model.h"

#include <QGraphicsView>
#include <QAction>
#include <QMenu>

namespace hal
{
    class GraphicsItem;
    class GraphWidget;
    class DragController;

    namespace graph_widget_constants
    {
        enum class grid_type;
    }

    class GraphGraphicsViewNeighborSelector : public GateSelectReceiver
    {
        Q_OBJECT

        u32 mOrigin;
        bool mPickSuccessor;
    public:
        GraphGraphicsViewNeighborSelector(u32 orig, bool pickSuc, QObject* parent = nullptr)
            : GateSelectReceiver(parent), mOrigin(orig), mPickSuccessor(pickSuc) {;}
    public Q_SLOTS:
        void handleGatesPicked(const QSet<u32>& gats) override;
    };

    /**
     * @ingroup graph-visuals
     * @brief A view to display the rendered graph (needs a GraphicsScene).
     *
     * The GraphGraphicsView is the GraphicsView for a GraphicsScene. (Its like the camera that shows a section of
     * the scene)
     */
    class GraphGraphicsView : public QGraphicsView
    {
        Q_OBJECT

        friend class GraphWidget;
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent GraphWidget
         */
        GraphGraphicsView(GraphWidget* parent);

        /**
         * Zoom in/out at the position of the mouse.
         *
         * @param factor - The new zoom factor
         */
        void gentleZoom(const qreal factor);

        /**
         * Zoom in/out at the center of the viewport.
         *
         * @param factor - The new zoom factor
         */
        void viewportCenterZoom(const qreal factor);

        GraphicsScene::GridType gridType();
        void setGridType(GraphicsScene::GridType gridType);

        Qt::KeyboardModifier dragModifier();
        void setDragModifier(Qt::KeyboardModifier dragModifier);

        Qt::KeyboardModifier panModifier();
        void setPanModifier(Qt::KeyboardModifier panModifier);

        void handleFoldModuleShortcut();
        void handleUnfoldModuleShortcut();

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted whenever the user double-clicks a module in the current view. <br>
         * Note that this signal is not emitted for double-clicks on gates or nets.
         *
         * @param id - The id of the module that was double clicked.
         */
        void moduleDoubleClicked(u32 id);

        /**
         * Q_SIGNAL that gets emitted to close all picker
         */
        void triggerTerminatePicker();

    public Q_SLOTS:
        /**
         * highlight shortest path between source gate and target node by putting the items on path into a new grouping
         *
         * @param idFrom - id of gate where path starts
         * @param idTo - id of gate where path ends
         */
        void handleShortestPathToGrouping(u32 idFrom, Node nodeTo, bool forwardDirection);

        /**
         * remove selected nodes from view
         */
        void handleRemoveFromView();

    private Q_SLOTS:
        void conditionalUpdate();
        void handleIsolationViewAction();
        void adjustMinScale();
        void handleAddCommentAction();

        void handleFoldParentSingle();
        void handleFoldParentAll();
        void handleUnfoldSingleAction();
        void handleUnfoldAllAction();

        void handleShortestPathToView();
        void handleQueryShortestPathGate();
        void handleQueryShortestPathModule();
        void handleSelectOutputs();
        void handleSelectInputs();

        void handleAddModuleToView();
        void handleAddGateToView();

        void handleAddSuccessorToView();
        void handleAddPredecessorToView();
        void handleAddCommonSuccessorToView();
        void handleAddCommonPredecessorToView();
        void handleHighlightSuccessor();
        void handleHighlightPredecessor();
        void handleSuccessorDistance();
        void handlePredecessorDistance();
        void handleCancelPickMode();
        void handlePluginContextContributionTriggered();
        void selectedNodeToItem();

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawForeground(QPainter* painter, const QRectF& rect) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragLeaveEvent(QDragLeaveEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void wheelEvent(QWheelEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:

        enum SearchAction {PredecessorGate, SuccessorGate, PredecessorModule, SuccessorModule};
        void mousePressEventNotItemDrag(QMouseEvent* event);
        void showContextMenu(const QPoint& pos);

        void updateMatrix(const int delta);

        void toggleAntialiasing();

        bool itemDraggable(GraphicsItem* item);

        void addSuccessorToView(int maxLevel, bool succ);
        void addCommonSuccessorToView(int maxLevel, bool succ);

        void dragPan(float dpx, float dpy);

        GraphWidget* mGraphWidget;

        QSet<u32> getSelectableGates();
        QSet<u32> getNotSelectableModules();

        struct LayouterPoint
        {
            int mIndex;
            qreal mPos;
        };
        QPair<QPoint, QPointF> closestLayouterPos(const QPointF& scene_pos) const;
        LayouterPoint closestLayouterPoint(qreal scene_pos, int default_spacing, int min_index, QVector<qreal> sections) const;

        #ifdef GUI_DEBUG_GRID
        void debugShowLayouterGridpos(const QPoint& mouse_pos);
        void debugDrawLayouterGridpos(QPainter* painter);
        QPoint m_debug_gridpos = QPoint(0,0);
        bool mDebugGridposEnable = true;
        #endif



        GraphicsItem* mItem;

        bool mMinimapEnabled;

        bool mGridEnabled;
        bool mGridClustersEnabled;
        GraphicsScene::GridType mGridType;

        DragController* mDragController;
        Qt::KeyboardModifier mDragModifier;

        QPoint mMovePosition;
        Qt::KeyboardModifier mPanModifier;

        Qt::KeyboardModifier mZoomModifier;
        qreal mZoomFactorBase;
        QPointF mTargetScenePos;
        QPointF mTargetViewportPos;

        qreal mMinScale;

        template <typename Func1>
        void recursionLevelMenu(QMenu* menu, bool succ, Func1 slot, bool addUnlimited=false)
        {
            QString s = succ ? "successors" : "predecessors";
            const char* txt[] = {"All %1", "Depth 1 (direct %1)", "Depth 2",
                                 "Depth 3", "Depth 4", "Depth 5", nullptr};
            for (int inx = 1; txt[inx]; inx++)
            {
                QAction* act = menu->addAction(inx==1 ? QString(txt[1]).arg(s) : QString(txt[inx]));
                act->setData(inx);
                connect(act, &QAction::triggered, this, slot);
            }
            if (addUnlimited)
            {
                QAction* act = menu->addAction(QString(txt[0]).arg(s));
                act->setData(0);
                connect(act, &QAction::triggered, this, slot);
            }
        }

        static const QString sAssignToGrouping;
    };
}

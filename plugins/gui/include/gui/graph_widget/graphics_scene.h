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

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"

#include <QGraphicsScene>
#include <QPair>
#include <QVector>

namespace hal
{
    class GraphicsGate;
    class GraphicsItem;
    class GraphicsModule;
    class GraphicsNet;

    /**
     * @ingroup graph
     * @brief Container for a GraphGraphicsView containing gates, nets, and modules.
     *
     * The GraphicsScene is the scene where the GraphicsItems of the gates, nets and modules are placed in by the layouter.
     * Besides the functions that are provided by its parent class QGraphicScene, it offers functions to work on a grid,
     * to support drag-and-drop on the grid, to access the contained GraphicItems and to handle certain events like
     * changed selections and groupings.
     */
    class GraphicsScene : public QGraphicsScene
    {
        Q_OBJECT

    public:
        enum GridType {None, Dots, Lines};
        Q_ENUM(GridType)
		
        /**
         * Sets the Level of Detail (LOD) of the scene. If the LOD falls below a certain threshold, the grid in the
         * background won't be drawn anymore.
         *
         * @param lod - The Level of Detail to draw
         */
        static void setLod(const qreal& lod);

        /**
         * Enables/Disables the grid in the background.
         *
         * @param value - <b>true</b> to enable the grid. <b>false</b> to disable the grid.
         */
        static void setGridEnabled(const bool& value);

        /**
         * Enables/Disables the grid clusters. The grid clusters are the darker lines in the grid that
         * indicate 8 steps in the main grid.
         *
         * @param value - <b>true</b> to enable the grid clusters. <b>false</b> to disable the grid clusters.
         */
        static void setGridClustersEnabled(const bool& value);

        /**
         * Sets the type of the grid. The following grid types are supported: None, Lines, Dots
         *
         * @param gridType - enumerated grid type
         */
        static void setGridType(const GridType& gridType);

        /**
         * Sets the color of the grid base lines (not the clusters). <br>
         * It does not affect the dot grid color.
         *
         * @param color - The color of the grid base lines
         */
        static void setGridBaseLineColor(const QColor& color);

        /**
         * Sets the color of the grid cluster lines. The grid cluster lines are the darker lines in the grid that
         * indicate 8 steps in the main grid. <br>
         * It does not affect the dot cluster grid color.
         *
         * @param color - The color of the grid cluster lines
         */
        static void setGridClusterLineColor(const QColor& color);

        /**
         * Sets the color of the grid base dotted lines (only shown if grid_type::Dots are activated). <br>
         *
         * @param color - The color of the grid base dotted lines
         */
        static void setGridBaseDotColor(const QColor& color);

        /**
         * Sets the color of the grid dotted cluster lines (only shown if grid_type::Dots are activated).
         * The grid cluster lines are the darker dots in the grid that indicate 8 steps in the main grid.
         *
         * @param color - The color of the grid dotted cluster lines
         */
        static void setGridClusterDotColor(const QColor& color);

        /**
         * Given any position, this function returns a position on the grid that is next to it.
         * \deprecated Please use GraphGraphicsView::closestLayouterPos instead.
         *
         * @param pos - The position to snap to the grid
         * @returns the closest position on the grid
         */
        static QPointF snapToGrid(const QPointF& pos) Q_DECL_DEPRECATED;

        /**
         * Constructor.
         *
         * @param parent - The parent QObject
         */
        GraphicsScene(QObject* parent = nullptr);

        /**
         * Destructor.
         *
         */
        ~GraphicsScene();

        /**
         * Starts the dragging of a gate or module to show its shadow meanwhile.
         *
         * @param posF - The position of the shadow
         * @param sizeF - The size of the shadow (i.e. the size of the dragged gate)
         * @param cue - The cue of the current position
         */
        void startDragShadow(const QPointF& posF, const QSizeF& sizeF, const NodeDragShadow::DragCue cue);

        /**
         * Moves the shadow that appears while dragging a gate or module.
         *
         * @param posF - The new position of the shadow
         * @param cue - The cue of the current position
         */
        void moveDragShadow(const QPointF& posF, const NodeDragShadow::DragCue cue);

        /**
         * Removes the shadow that appears while dragging a gate or module (at the end of the drag action).
         */
        void stopDragShadow();

        /**
         * Gets the position of the drag shadow.
         *
         * @returns the position of the drag shadow.
         */
        QPointF dropTarget();

        /**
         * Adds a GraphicsItem to the scene.
         *
         * @param item - The GraphicsItem to add
         */
        void addGraphItem(GraphicsItem* item);

        /**
         * Removes a GraphicsItem from the scene.
         *
         * @param item - The GraphicsItem to remove
         */
        void removeGraphItem(GraphicsItem* item);

        /**
         * Removes all GraphicsItems from the scene.
         */
        void deleteAllItems();

        /**
         * Connects all necessary signals and slots.
         */
        void connectAll();

        /**
         * Disconnects all connected signals and slots.
         */
        void disconnectAll();

        /**
         * Updates all visuals using a certain GraphShader.
         *
         * @param s - The used GraphShader
         */
        void updateVisuals(const GraphShader::Shading& s);

        /**
         * Moves all GraphicsNets to the background so that they do not overlap GraphicsGates.
         * Helper function that can be used by the layouters.
         */
        void moveNetsToBackground();

        /**
         * Gets the GraphicsGate for the corresponding gate. If there is no GraphicsGate for the given id,
         * a <i>nullptr</i> is returned.
         *
         * @param id - The id of the gate in the netlist
         * @returns the GraphicsGate.
         */
        const GraphicsGate* getGateItem(const u32 id) const;

        /**
         * Gets the GraphicsNet for the corresponding net. If there is no GraphicsNet for the given id,
         * a <i>nullptr</i> is returned.
         *
         * @param id - The id of the net in the netlist
         * @returns the GraphicsNet.
         */
        const GraphicsNet* getNetItem(const u32 id) const;

        /**
         * Gets the GraphicsModule for the corresponding module. If there is no GraphicsModule for the given id,
         * a <i>nullptr</i> is returned.
         *
         * @param id - The id of the module in the netlist
         * @returns the GraphicsModule.
         */
        const GraphicsModule* getModuleItem(const u32 id) const;

        #ifdef GUI_DEBUG_GRID
        void debugSetLayouterGrid(const QVector<qreal>& debug_x_lines, const QVector<qreal>& debug_y_lines, qreal debug_default_height, qreal debug_default_width);
        void setDebugGridEnabled(bool enabled);
        bool debugGridEnabled();
        #endif

    public Q_SLOTS:
        /**
         * Q_SLOT to call when the selection has been changed. Used to keep track of the selected modules, gates and nets.
         */
        void handleInternSelectionChanged();

        /**
         * Q_SLOT to call whenever the selection has beed changed. It only handles selection changes outside this scene.
         * It is used to apply extern selection changes to the scene.
         *
         * @param sender - The object that is responsible for the changes and called the signal.
         */
        void handleExternSelectionChanged(void* sender);

        /**
         * Q_SLOT to call whenever the sub-focus has been changed. Currently unused.
         *
         * @param sender
         */
        void handleExternSubfocusChanged(void* sender);

        /**
         * Q_SLOT used to highlight the selected graphics items.
         *
         * @param highlightItems - The selection tree items to highlight
         */
        void handleHighlight(const QVector<const SelectionTreeItem*>& highlightItems);

        /**
         * Q_SLOT to call whenever a module was assigned to or removed from a grouping. It is used to update the
         * corresponding GraphicsModule accordingly.
         *
         * @param grp - The grouping that has been changed
         * @param id - The id of the module that was assigned/removed to/from the grouping
         */
        void handleGroupingAssignModule(Grouping* grp, u32 id);

        /**
         * Q_SLOT to call whenever a gate was assigned to or removed from a grouping. It is used to update the
         * corresponding GraphicsGate accordingly.
         *
         * @param grp - The grouping that has been changed
         * @param id - The id of the gate that was assigned/removed to/from the grouping
         */
        void handleGroupingAssignGate(Grouping* grp, u32 id);

        /**
         * Q_SLOT to call whenever a net was assigned to or removed from a grouping. It is used to update the
         * corresponding GraphicsNet accordingly.
         *
         * @param grp - The grouping that has been changed
         * @param id - The id of the net that was assigned/removed to/from the grouping
         */
        void handleGroupingAssignNet(Grouping* grp, u32 id);

        /**
         * Q_SLOT to call whenever the color of a grouping has been changed. It is used to update the affected
         * GraphicsItems accordingly.
         *
         * @param grp - The grouping that has been changed
         */
        void handleGroupingColorChanged(Grouping* grp);

    protected:
        /**
         * Handles the mouse event. Used to intercept and ignore right-clicks.
         *
         * @param event - The mouse event in the graphics scene
         */
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        struct ModuleData
        {
            u32 mId;
            GraphicsModule* mItem;
        };

        struct GateData
        {
            u32 mId;
            GraphicsGate* mItem;
        };

        struct NetData
        {
            u32 mId;
            GraphicsNet* mItem;
        };

        static qreal sLod;

        static const qreal sGridFadeStart;
        static const qreal sGridFadeEnd;

        static bool sGridEnabled;
        static bool sGridClustersEnabled;
        static GraphicsScene::GridType sGridType;

        static QColor sGridBaseLineColor;
        static QColor sGridClusterLineColor;

        static QColor sGridBaseDotColor;
        static QColor sGridClusterDotColor;

        using QGraphicsScene::addItem;
        using QGraphicsScene::removeItem;
        using QGraphicsScene::clear;

        void drawBackground(QPainter* painter, const QRectF& rect) override;

        NodeDragShadow* mDragShadowGate;

        QVector<ModuleData> mModuleItems;
        QVector<GateData> mGateItems;
        QVector<NetData> mNetItems;

        #ifdef GUI_DEBUG_GRID
        void debugDrawLayouterGrid(QPainter* painter, const int x_from, const int x_to, const int y_from, const int y_to);
        QVector<qreal> mDebugXLines;
        QVector<qreal> mDebugYLines;
        qreal mDebugDefaultWidth;
        qreal mDebugDefaultHeight;
        bool mDebugGridEnable;
        #endif
    };
}

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

    namespace graph_widget_constants
    {
    enum class grid_type;
    }

    class GraphicsScene : public QGraphicsScene
    {
        Q_OBJECT

    public:
        static void setLod(const qreal& lod);
        static void setGridEnabled(const bool& value);
        static void setGridClustersEnabled(const bool& value);
        static void setGridType(const graph_widget_constants::grid_type& grid_type);

        static void setGridBaseLineColor(const QColor& color);
        static void setGridClusterLineColor(const QColor& color);
        static void setGridBaseDotColor(const QColor& color);
        static void setGridClusterDotColor(const QColor& color);

        static QPointF snapToGrid(const QPointF& pos) Q_DECL_DEPRECATED;

        GraphicsScene(QObject* parent = nullptr);

        void startDragShadow(const QPointF& posF, const QSizeF& sizeF, const NodeDragShadow::DragCue cue);
        void moveDragShadow(const QPointF& posF, const NodeDragShadow::DragCue cue);
        void stopDragShadow();
        QPointF dropTarget();

        void addGraphItem(GraphicsItem* item);
        void removeGraphItem(GraphicsItem* item);

        void deleteAllItems();

        void connectAll();
        void disconnectAll();

        void updateVisuals(const GraphShader::Shading& s);

        void moveNetsToBackground();

        const GraphicsGate* getGateItem(const u32 id) const;
        const GraphicsNet* getNetItem(const u32 id) const;
        const GraphicsModule* getModuleItem(const u32 id) const;

        #ifdef GUI_DEBUG_GRID
        void debugSetLayouterGrid(const QVector<qreal>& debug_x_lines, const QVector<qreal>& debug_y_lines, qreal debug_default_height, qreal debug_default_width);
        #endif

    public Q_SLOTS:
        void handleInternSelectionChanged();
        void handleExternSelectionChanged(void* sender);
        void handleExternSubfocusChanged(void* sender);
        void handleHighlight(const QVector<const SelectionTreeItem*>& highlightItems);
        void handleGroupingAssignModule(Grouping* grp, u32 id);
        void handleGroupingAssignGate(Grouping* grp, u32 id);
        void handleGroupingAssignNet(Grouping* grp, u32 id);
        void handleGroupingColorChanged(Grouping* grp);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private Q_SLOTS:
        void handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value);

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
        static graph_widget_constants::grid_type sGridType;

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

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
        static void set_lod(const qreal& lod);
        static void set_grid_enabled(const bool& value);
        static void set_grid_clusters_enabled(const bool& value);
        static void set_grid_type(const graph_widget_constants::grid_type& grid_type);

        static void set_grid_base_line_color(const QColor& color);
        static void set_grid_cluster_line_color(const QColor& color);
        static void set_grid_base_dot_color(const QColor& color);
        static void set_grid_cluster_dot_color(const QColor& color);

        static QPointF snap_to_grid(const QPointF& pos) Q_DECL_DEPRECATED;

        GraphicsScene(QObject* parent = nullptr);

        void start_drag_shadow(const QPointF& posF, const QSizeF& sizeF, const NodeDragShadow::drag_cue cue);
        void move_drag_shadow(const QPointF& posF, const NodeDragShadow::drag_cue cue);
        void stop_drag_shadow();
        QPointF drop_target();

        void add_item(GraphicsItem* item);
        void remove_item(GraphicsItem* item);

        void delete_all_items();

        void connect_all();
        void disconnect_all();

        void update_visuals(const GraphShader::shading& s);

        void move_nets_to_background();

        const GraphicsGate* get_gate_item(const u32 id) const;
        const GraphicsNet* get_net_item(const u32 id) const;
        const GraphicsModule* get_ModuleItem(const u32 id) const;

        #ifdef GUI_DEBUG_GRID
        void debug_set_layouter_grid(const QVector<qreal>& debug_x_lines, const QVector<qreal>& debug_y_lines, qreal debug_default_height, qreal debug_default_width);
        #endif

    public Q_SLOTS:
        void handle_intern_selection_changed();
        void handle_extern_selection_changed(void* sender);
        void handle_extern_subfocus_changed(void* sender);
        void handleHighlight(const QVector<const SelectionTreeItem*>& highlightItems);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private Q_SLOTS:
        void handle_global_setting_changed(void* sender, const QString& key, const QVariant& value);

    private:
        struct module_data
        {
            u32 id;
            GraphicsModule* item;
        };

        struct gate_data
        {
            u32 id;
            GraphicsGate* item;
        };

        struct net_data
        {
            u32 id;
            GraphicsNet* item;
        };

        static qreal s_lod;

        static const qreal s_grid_fade_start;
        static const qreal s_grid_fade_end;

        static bool s_grid_enabled;
        static bool s_grid_clusters_enabled;
        static graph_widget_constants::grid_type s_grid_type;

        static QColor s_grid_base_line_color;
        static QColor s_grid_cluster_line_color;

        static QColor s_grid_base_dot_color;
        static QColor s_grid_cluster_dot_color;

        using QGraphicsScene::addItem;
        using QGraphicsScene::removeItem;
        using QGraphicsScene::clear;

        void drawBackground(QPainter* painter, const QRectF& rect) override;

        NodeDragShadow* m_drag_shadow_gate;

        QVector<module_data> m_ModuleItems;
        QVector<gate_data> m_gate_items;
        QVector<net_data> m_net_items;

        #ifdef GUI_DEBUG_GRID
        void debug_draw_layouter_grid(QPainter* painter, const int x_from, const int x_to, const int y_from, const int y_to);
        QVector<qreal> m_debug_x_lines;
        QVector<qreal> m_debug_y_lines;
        qreal m_debug_default_width;
        qreal m_debug_default_height;
        bool m_debug_grid_enable;
        #endif
    };
}

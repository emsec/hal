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
#include "gui/gui_globals.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include <QGraphicsView>
#include <QAction>

namespace hal
{
    class GraphicsItem;
    class GraphWidget;

    namespace graph_widget_constants
    {
    enum class grid_type;
    }

    class GraphGraphicsView : public QGraphicsView
    {
        Q_OBJECT

    public:
        GraphGraphicsView(GraphWidget* parent);

        //zooms into the mouse position
        void gentle_zoom(const qreal factor);
        //zooms into the center of the viewport
        void viewport_center_zoom(const qreal factor);

    Q_SIGNALS:
        void module_double_clicked(u32 id);

    private Q_SLOTS:
        void conditional_update();
        void handle_change_color_action();
        void handle_isolation_view_action();
        void handle_move_action(QAction* action);
        void handle_move_new_action();
        void handle_rename_action();
        void adjust_min_scale();

        void handle_fold_single_action();
        void handle_fold_all_action();
        void handle_unfold_single_action();
        void handle_unfold_all_action();

        void handle_select_outputs();
        void handle_select_inputs();

        void handle_global_setting_changed(void* sender, const QString& key, const QVariant& value);

    private:
        void paintEvent(QPaintEvent* event) override;
        void drawForeground(QPainter* painter, const QRectF& rect) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragLeaveEvent(QDragLeaveEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void wheelEvent(QWheelEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

        void initialize_settings();

        void show_context_menu(const QPoint& pos);

        void update_matrix(const int delta);

        void toggle_antialiasing();

        bool item_draggable(GraphicsItem* item);

        struct layouter_point
        {
            int index;
            qreal pos;
        };
        QVector<QPoint> closest_layouter_pos(const QPointF& scene_pos) const;
        layouter_point closest_layouter_point(qreal scene_pos, int default_spacing, int min_index, QVector<qreal> sections) const;

        #ifdef GUI_DEBUG_GRID
        void debug_show_layouter_gridpos(const QPoint& mouse_pos);
        void debug_draw_layouter_gridpos(QPainter* painter);
        QPoint m_debug_gridpos = QPoint(0,0);
        bool m_debug_gridpos_enable;
        #endif

        GraphWidget* m_graph_widget;

        GraphicsItem* m_item;

        bool m_minimap_enabled;

        bool m_grid_enabled;
        bool m_grid_clusters_enabled;
        graph_widget_constants::grid_type m_grid_type;

        QPoint m_drag_mousedown_position;
        QPoint m_drag_start_gridpos;
        GraphicsGate* m_drag_item;
        QPoint m_drag_current_gridpos;
        bool m_drag_current_modifier;
        bool m_drop_allowed;

        Qt::KeyboardModifier m_drag_modifier;

        QPoint m_move_position;
        Qt::KeyboardModifier m_move_modifier;

        Qt::KeyboardModifier m_zoom_modifier;
        qreal m_zoom_factor_base;
        QPointF m_target_scene_pos;
        QPointF m_target_viewport_pos;

        qreal m_min_scale;
    };
}

#include "graph_widget/graph_graphics_view.h"

#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_items/global_graphics_net.h"
#include "gui/graph_widget/graphics_items/graphics_gate.h"
#include "gui/graph_widget/graphics_items/graphics_item.h"
#include "gui/graph_widget/graphics_items/separated_graphics_net.h"
#include "gui/graph_widget/graphics_items/standard_graphics_net.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/gui_globals.h"

#include <QAction>
#include <QColorDialog>
#include <qmath.h>
#include <QMenu>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>

graph_graphics_view::graph_graphics_view(QWidget* parent) : QGraphicsView(parent),
    m_minimap_enabled(false),
    m_antialiasing_enabled(false),
    m_cosmetic_nets_enabled(false),
    m_grid_enabled(true),
    m_grid_clusters_enabled(true),
    m_grid_type(graph_widget_constants::grid_type::lines)
{
    connect(this, &graph_graphics_view::customContextMenuRequested, this, &graph_graphics_view::show_context_menu);

    //    connect(&g_selection_relay, &selection_relay::focus_update, this, &graph_graphics_view::conditional_update);
    //    connect(&g_selection_relay, &selection_relay::subfocus_update, this, &graph_graphics_view::conditional_update);

    connect(&g_selection_relay, &selection_relay::subfocus_changed, this, &graph_graphics_view::conditional_update);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void graph_graphics_view::conditional_update()
{
    if (QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform()) >= graph_widget_constants::gate_min_lod)
        update();
}

void graph_graphics_view::handle_change_color_action()
{
    QColor color = QColorDialog::getColor();

    if (!color.isValid())
        return;
}

void graph_graphics_view::handle_cone_view_action()
{
    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_item->id());

    if (!g)
        return;
}

void graph_graphics_view::adjust_min_scale()
{
    if (!scene())
        return;

   m_min_scale = std::min(viewport()->width() / scene()->width(), viewport()->height() / scene()->height());
}

void graph_graphics_view::paintEvent(QPaintEvent* event)
{
    qreal lod = QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform());

    // USE CONSISTENT METHOD NAMES
    graphics_scene::set_lod(lod);
    graphics_scene::set_grid_enabled(m_grid_enabled);
    graphics_scene::set_grid_clusters_enabled(m_grid_clusters_enabled);
    graphics_scene::set_grid_type(m_grid_type);

    graphics_item::set_lod(lod);

    separated_graphics_net::update_alpha();
    global_graphics_net::update_alpha();

    QGraphicsView::paintEvent(event);
}

void graph_graphics_view::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    graphics_item* item = static_cast<graphics_item*>(itemAt(event->pos()));

    if (!item)
        return;

    if(item->get_item_type() == graphics_item::item_type::module)
        Q_EMIT module_double_clicked(item->id());
}

void graph_graphics_view::drawForeground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    if (!m_minimap_enabled)
        return;

//    QRectF bar(0, 0, viewport()->width(), 30);
//    painter->resetTransform();
//    painter->fillRect(bar, QColor(0, 0, 0, 170));

    QRectF map(viewport()->width() - 210, viewport()->height() - 130, 200, 120);
    painter->resetTransform();
    painter->fillRect(map, QColor(0, 0, 0, 170));
}

void graph_graphics_view::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ShiftModifier)
    {
        if (event->button() == Qt::LeftButton)
            m_move_position = event->pos();
    }
    else
        if (event->button() == Qt::MidButton)
        {
            m_zoom_position = event->pos();
            m_zoom_scene_position = mapToScene(event->pos());

            // HIDE CURSOR
            // SHOW DUMMY ???
            // ZOOM
            // MOVE ACTUAL CURSOR TO FINAL POSITION
            // SHOW ACTUAL CURSOR

            QCursor cursor(Qt::BlankCursor);
            setCursor(cursor);
        }
        else
            QGraphicsView::mousePressEvent(event);
}

void graph_graphics_view::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton)
    {
        QCursor cursor;
        cursor.setPos(mapToGlobal(mapFromScene(m_zoom_scene_position)));
        setCursor(cursor);
    }
    else
        QGraphicsView::mouseReleaseEvent(event);
}

void graph_graphics_view::mouseMoveEvent(QMouseEvent* event)
{
    if (!scene())
        return;

    if (event->modifiers() == Qt::ShiftModifier)
    {
        if (event->buttons().testFlag(Qt::LeftButton))
        {
            QScrollBar *hBar = horizontalScrollBar();
            QScrollBar *vBar = verticalScrollBar();
            QPoint delta = event->pos() - m_move_position;
            m_move_position = event->pos();
            hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
            vBar->setValue(vBar->value() - delta.y());
        }
    }
    else if (event->buttons().testFlag(Qt::MidButton))
    {
        int delta = m_zoom_position.y() - event->pos().y();

        if (delta)
            delta = std::min(delta, 100);
        else
            delta = std::max(delta, -100);

        update_matrix(delta);
        centerOn(m_zoom_scene_position);
    }
    else
        QGraphicsView::mouseMoveEvent(event);
}

void graph_graphics_view::wheelEvent(QWheelEvent* event)
{
    if (event->delta() > 0)
        Q_EMIT zoomed_in(6);
    else
        Q_EMIT zoomed_out(6);

    event->accept();
}

void graph_graphics_view::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        setInteractive(false);
        this->setDragMode(QGraphicsView::ScrollHandDrag);
    }

    switch (event->key())
    {
        case Qt::Key_Space:
        {
            //qDebug() << "Space pressed";
        }
        break;
    }

    event->ignore();

    //    QList<QGraphicsItem*> items = this->scene()->selectedItems();
    //    if (items.size() > 1 || items.size() == 0)
    //        return;
    //    gui_graph_gate* selected_gui_gate = dynamic_cast<gui_graph_gate*>(items.at(0));

    //    switch (state)
    //    {
    //        case gate_selected:
    //            gate_selected_key_pressed(event, selected_gui_gate);
    //            break;
    //        case gate_input_pins_selected:
    //            gate_left_pins_selected_key_pressed(event, selected_gui_gate);
    //            break;
    //        case gate_output_pins_selected:
    //            gate_right_pins_selected_key_pressed(event, selected_gui_gate);
    //            break;
    //        case net_selected:
    //            net_selected_key_pressed(event);
    //            break;
    //        case no_gate_selected:
    //            break;    // added to suppress warning
    //    }
}

void graph_graphics_view::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        setInteractive(true);
        this->setDragMode(QGraphicsView::RubberBandDrag);
    }
}

void graph_graphics_view::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)

    adjust_min_scale();
}

void graph_graphics_view::show_context_menu(const QPoint& pos)
{
    graphics_scene* s = static_cast<graphics_scene*>(scene());

    if (!s)
        return;

    QMenu context_menu(this);

    QGraphicsItem* item = itemAt(pos);
    if (item)
    {
        m_item = static_cast<graphics_item*>(item);

        switch (m_item->get_item_type())
        {
            case graphics_item::item_type::gate:
            {
                QAction* color_action = context_menu.addAction("Change Color");
                QObject::connect(color_action, &QAction::triggered, this, &graph_graphics_view::handle_change_color_action);
                context_menu.addAction(color_action);

                QAction* cone_view_action = context_menu.addAction("Open in Cone View");
                QObject::connect(cone_view_action, &QAction::triggered, this, &graph_graphics_view::handle_cone_view_action);
                context_menu.addAction(cone_view_action);
                break;
            }
            default:
                break;
        }
    }
    else
    {
        QAction* antialiasing_action = context_menu.addAction("Antialiasing");
        QAction* cosmetic_action = context_menu.addAction("Cosmetic Nets");
        QMenu* grid_menu = context_menu.addMenu("Grid");
        QMenu* type_menu = grid_menu->addMenu("Type");
        QMenu* cluster_menu = grid_menu->addMenu("Clustering");
        QAction* lines_action = type_menu->addAction("Lines");
        QAction* dots_action = type_menu->addAction("Dots");
        QAction* none_action = type_menu->addAction("None");
        //connect(action, &QAction::triggered, this, SLOT);
    }

    context_menu.exec(mapToGlobal(pos));
    update();
}

void graph_graphics_view::update_matrix(const int delta)
{
    qreal scale = qPow(qreal(2), (delta) / qreal(100));

    QMatrix matrix;
    matrix.scale(scale, scale);
    setMatrix(matrix);
}


void graph_graphics_view::toggle_antialiasing()
{
    //m_antialiasing_enabled ? setRenderHint(QPainter::Antialiasing, false) : setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::Antialiasing, !(renderHints() & QPainter::Antialiasing));
}

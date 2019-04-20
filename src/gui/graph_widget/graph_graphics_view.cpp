#include "graph_widget/graph_graphics_view.h"
#include "graph_widget/graph_layouter_manager.h"
#include "graph_widget/graph_scene.h"
#include "graph_widget/graph_widget.h"
#include "graph_widget/graphics_items/graph_graphics_item.h"
#include "graph_widget/graphics_items/graphics_gate.h"
#include "graph_widget/graphics_items/standard_graphics_net.h"
#include <QWheelEvent>

graph_graphics_view::graph_graphics_view(graph_widget* widget) : m_widget(widget)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    //    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //setCacheMode(QGraphicsView::CacheBackground);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    //setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setMouseTracking(false);

    connect(this, &graph_graphics_view::customContextMenuRequested, this, &graph_graphics_view::show_context_menu);
}

void graph_graphics_view::handle_change_color_action()
{
    QColor color = QColorDialog::getColor();

    if (color.isValid())
        m_item->set_color(color);
}

void graph_graphics_view::paintEvent(QPaintEvent* event)
{
    graph_graphics_item::set_lod(QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform()));
    QGraphicsView::paintEvent(event);
}

void graph_graphics_view::wheelEvent(QWheelEvent* e)
{
    //    if (e->modifiers() & Qt::ControlModifier)
    //        QGraphicsView::wheelEvent(e);
    //    else
    //    {
    if (e->delta() > 0)
        m_widget->zoom_in(6);
    else
        m_widget->zoom_out(6);
    e->accept();
    //    }
}

void graph_graphics_view::show_context_menu(const QPoint& pos)
{
    QGraphicsItem* item = itemAt(pos);
    if (item)
    {
        QMenu contextMenu(this);
        //graph_graphics_item* graphics_item = static_cast<graph_graphics_item>(item);
        m_item = static_cast<graph_graphics_item*>(item);

        //        switch(graphics_item->get_class())
        //        {
        //        case item_class::node:
        //        }

        QAction colorAction("Change Color");
        QObject::connect(&colorAction, &QAction::triggered, this, &graph_graphics_view::handle_change_color_action);
        contextMenu.addAction(&colorAction);
        contextMenu.exec(mapToGlobal(pos));

        //        switch (item->type()) {
        //            case QGraphicsItem::UserType + 2: {
        ////                m_context_menu_node = dynamic_cast<graph_node_ui *>(item);
        //                QAction action1("Change Color", this);
        //                connect(&action1, &QAction::triggered, this, &graph_graphics_widget::change_node_color);
        //                contextMenu.addAction(&action1); //Placeholder

        //                contextMenu.exec(mapToGlobal(pos));
        //                break;
        //            }
        //            default: {
        //                QAction action2("QGraphicsItem", this);
        //                connect(&action2, SIGNAL(triggered()), this, SLOT(placeholder()));
        //                contextMenu.addAction(&action2); //Placeholder

        //                contextMenu.exec(mapToGlobal(pos));
        //            }
        //        }
    }
    //    else
    //    {
    //        QMenu contextMenu(tr("Context menu"), this);
    //        if (m_background_grid->grid_visible())
    //        {
    //            QAction* action = new QAction("hide Grid", this);
    //            connect(action, SIGNAL(triggered()), m_background_grid, SLOT(set_draw_grid_false()));
    //            contextMenu.addAction(action);
    //        }
    //        else
    //        {
    //            QAction* action = new QAction("show Grid", this);
    //            connect(action, SIGNAL(triggered()), m_background_grid, SLOT(set_draw_grid_true()));
    //            contextMenu.addAction(action);
    //        }
    //        contextMenu.exec(mapToGlobal(pos));
    //    }
}

void graph_graphics_view::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        setInteractive(false);
        this->setDragMode(QGraphicsView::ScrollHandDrag);
    }
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

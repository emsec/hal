#include "graph_widget/graphics_items/graph_graphics_item.h"
#include <QtWidgets>

qreal graph_graphics_item::s_lod;

QColor graph_graphics_item::s_selection_color;

void graph_graphics_item::load_settings()
{
    s_selection_color = Qt::red;
}

void graph_graphics_item::set_lod(qreal lod)
{
    s_lod = lod;
}

graph_graphics_item::graph_graphics_item()
{
    m_color = Qt::lightGray;

    setFlags(ItemIsSelectable);
    //setAcceptHoverEvents(true);
}

item_class graph_graphics_item::get_class()
{
    return m_class;
}

void graph_graphics_item::set_color(QColor& color)
{
    m_color = color;
    update(boundingRect());
}

void graph_graphics_item::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    update(boundingRect());
}

void graph_graphics_item::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    //jump
    update(boundingRect());
}

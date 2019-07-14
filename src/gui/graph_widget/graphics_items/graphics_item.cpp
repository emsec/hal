#include "gui/graph_widget/graphics_items/graphics_item.h"

qreal graphics_item::s_lod;

QColor graphics_item::s_selection_color;

void graphics_item::load_settings()
{
    s_selection_color = Qt::red;
}

void graphics_item::set_lod(const qreal lod)
{
    s_lod = lod;
}

graphics_item::graphics_item(const item_type type, const u32 id) :
    m_class(type),
    m_id(id)
{
    m_color = Qt::lightGray;

    setFlags(ItemIsSelectable | ItemIsFocusable);
    //setAcceptHoverEvents(true);
}

u32 graphics_item::id() const
{
    return m_id;
}

graphics_item::item_type graphics_item::get_item_type() const
{
    return m_class;
}

void graphics_item::set_color(const QColor& color)
{
    m_color = color;
    update(boundingRect());
}

void graphics_item::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    update(boundingRect());
}

void graphics_item::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    // PROBABLY NO LONGER NEEDED HERE
    update(boundingRect());
}

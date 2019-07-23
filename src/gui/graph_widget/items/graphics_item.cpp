#include "gui/graph_widget/items/graphics_item.h"

qreal graphics_item::s_lod;

QColor graphics_item::s_selection_color;

void graphics_item::load_settings()
{
    s_selection_color = QColor(240, 173, 0);
}

void graphics_item::set_lod(const qreal lod)
{
    s_lod = lod;
}

graphics_item::graphics_item(const hal::item_type type, const u32 id) :
    m_type(type),
    m_id(id)
{
    m_color = Qt::lightGray;

    setFlags(ItemIsSelectable);
    //setFlags(ItemIsSelectable | ItemIsFocusable);
}

u32 graphics_item::id() const
{
    return m_id;
}

hal::item_type graphics_item::get_item_type() const
{
    return m_type;
}

void graphics_item::set_color(const QColor& color)
{
    m_color = color;
    update(boundingRect());
}

//void graphics_item::mousePressEvent(QGraphicsSceneMouseEvent* event)
//{
//    QGraphicsItem::mousePressEvent(event);
//    update(boundingRect());
//}

//void graphics_item::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
//{
//    QGraphicsItem::mouseDoubleClickEvent(event);
//    update(boundingRect());
//}

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
    m_item_type(type),
    m_id(id),
    m_color(255, 0, 230)
{
    m_color = Qt::lightGray; // REMOVE LATER
    setFlags(ItemIsSelectable);
}

hal::item_type graphics_item::item_type() const
{
    return m_item_type;
}

u32 graphics_item::id() const
{
    return m_id;
}

void graphics_item::set_color(const QColor& color)
{
    m_color = color;
    update();
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

#include "gui/graph_widget/items/graphics_item.h"

namespace hal
{
    qreal GraphicsItem::s_lod;
    QColor GraphicsItem::s_selection_color;

    void GraphicsItem::load_settings()
    {
        s_selection_color = QColor(240, 173, 0);
    }

    void GraphicsItem::set_lod(const qreal lod)
    {
        s_lod = lod;
    }

    GraphicsItem::GraphicsItem(const hal::item_type type, const u32 id) :
        m_item_type(type),
        m_id(id),
        m_color(255, 0, 255)
    {
        setFlags(ItemIsSelectable);
    }

    hal::item_type GraphicsItem::item_type() const
    {
        return m_item_type;
    }

    u32 GraphicsItem::id() const
    {
        return m_id;
    }
}

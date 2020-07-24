#include "gui/graph_widget/items/graphics_item.h"

namespace hal
{
    qreal GraphicsItem::s_lod;
    QColor GraphicsItem::s_selectionColor;
    QColor GraphicsItem::s_highlightColor;

    void GraphicsItem::load_settings()
    {
        s_selectionColor = QColor(240, 173, 0);
        s_highlightColor = QColor(40, 200, 240);
    }

    void GraphicsItem::set_lod(const qreal lod)
    {
        s_lod = lod;
    }

    GraphicsItem::GraphicsItem(const hal::item_type type, const u32 id) :
        m_item_type(type),
        m_highlight(false),
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

    void GraphicsItem::setHightlight(bool hl)
    {
        if (hl==m_highlight) return;  // nothing to do
        m_highlight = hl;
        update();
    }

    QColor GraphicsItem::penColor(QStyle::State state) const
    {
        if (m_highlight) return s_highlightColor;
        if (state & QStyle::State_Selected) return s_selectionColor;
        return m_color;
    }
}

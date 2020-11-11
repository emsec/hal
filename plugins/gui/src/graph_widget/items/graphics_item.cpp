#include "gui/graph_widget/items/graphics_item.h"
#include "gui/gui_globals.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"

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

    GraphicsItem::GraphicsItem(const ItemType type, const u32 id) :
        m_item_type(type),
        m_highlight(false),
        m_id(id),
        m_color(255, 0, 255)
    {
        setFlags(ItemIsSelectable);
    }

    ItemType GraphicsItem::item_type() const
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

    QColor GraphicsItem::selectionColor() const
    {
        if (m_highlight) return s_highlightColor;
        return s_selectionColor;
    }

    QColor GraphicsItem::groupingColor() const
    {
        return g_content_manager->getGroupingManagerWidget()->getModel()->colorForItem(m_item_type,m_id);
    }

    QColor GraphicsItem::penColor(QStyle::State state, const QColor& colorHint) const
    {
        if (state & QStyle::State_Selected) return selectionColor();
        QColor gcol = groupingColor();
        if (gcol.isValid()) return gcol;
        if (colorHint.isValid()) return colorHint;
        return m_color;
    }
}

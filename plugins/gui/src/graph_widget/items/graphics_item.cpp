#include "gui/graph_widget/items/graphics_item.h"
#include "gui/gui_globals.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"

namespace hal
{
    qreal GraphicsItem::sLod;
    QColor GraphicsItem::sSelectionColor;
    QColor GraphicsItem::sHighlightColor;

    void GraphicsItem::loadSettings()
    {
        sSelectionColor = QColor(240, 173, 0);
        sHighlightColor = QColor(40, 200, 240);
    }

    void GraphicsItem::setLod(const qreal lod)
    {
        sLod = lod;
    }

    GraphicsItem::GraphicsItem(const ItemType type, const u32 id) :
        mItemType(type),
        mHighlight(false),
        mId(id),
        mColor(255, 0, 255)
    {
        setFlags(ItemIsSelectable);
    }

    ItemType GraphicsItem::itemType() const
    {
        return mItemType;
    }

    u32 GraphicsItem::id() const
    {
        return mId;
    }

    void GraphicsItem::setHightlight(bool hl)
    {
        if (hl==mHighlight) return;  // nothing to do
        mHighlight = hl;
        update();
    }

    QColor GraphicsItem::selectionColor() const
    {
        if (mHighlight) return sHighlightColor;
        return sSelectionColor;
    }

    QColor GraphicsItem::groupingColor() const
    {
        return gContentManager->getGroupingManagerWidget()->getModel()->colorForItem(mItemType,mId);
    }

    QColor GraphicsItem::penColor(QStyle::State state, const QColor& colorHint) const
    {
        if (state & QStyle::State_Selected) return selectionColor();
        QColor gcol = groupingColor();
        if (gcol.isValid()) return gcol;
        if (colorHint.isValid()) return colorHint;
        return mColor;
    }
}

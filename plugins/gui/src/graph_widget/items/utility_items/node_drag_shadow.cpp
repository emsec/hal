#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"

#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include <assert.h>

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>

namespace hal
{
    QPen NodeDragShadow::sPen;
    qreal NodeDragShadow::sLod;

    QColor NodeDragShadow::sColorPen[3];
    QColor NodeDragShadow::sColorSolid[3];
    QColor NodeDragShadow::sColorTranslucent[3];

    void NodeDragShadow::loadSettings()
    {
        sColorPen[static_cast<int>(DragCue::Rejected)] = QColor(166, 31, 31, 255);
        sColorPen[static_cast<int>(DragCue::Movable)] = QColor(48, 172, 79, 255);
        sColorPen[static_cast<int>(DragCue::Swappable)] = QColor(37, 97, 176, 255);

        sColorSolid[static_cast<int>(DragCue::Rejected)] = QColor(166, 31, 31, 200);
        sColorSolid[static_cast<int>(DragCue::Movable)] = QColor(48, 172, 79, 200);
        sColorSolid[static_cast<int>(DragCue::Swappable)] = QColor(37, 97, 176, 200);

        sColorTranslucent[static_cast<int>(DragCue::Rejected)] = QColor(166, 31, 31, 150);
        sColorTranslucent[static_cast<int>(DragCue::Movable)] = QColor(48, 172, 79, 150);
        sColorTranslucent[static_cast<int>(DragCue::Swappable)] = QColor(37, 97, 176, 150);

        sPen.setCosmetic(true);
        sPen.setJoinStyle(Qt::MiterJoin);
    }

    NodeDragShadow::NodeDragShadow() : QGraphicsObject()
    {
        hide();

        setAcceptedMouseButtons(0);
        mWidth = 100;
        mHeight = 100;
    }

    void NodeDragShadow::start(const QPointF& posF, const QSizeF& sizeF)
    {
        setPos(posF);
        setWidth(sizeF.width());
        setHeight(sizeF.height());
        setZValue(1);
        show();
    }

    void NodeDragShadow::stop()
    {
        hide();
    }

    qreal NodeDragShadow::width() const
    {
        return mWidth;
    }

    qreal NodeDragShadow::height() const
    {
        return mHeight;
    }

    QSizeF NodeDragShadow::size() const
    {
        return QSizeF(mWidth, mHeight);
    }

    void NodeDragShadow::setWidth(const qreal width)
    {
        mWidth = width;
    }

    void NodeDragShadow::setHeight(const qreal height)
    {
        mHeight = height;
    }

    void NodeDragShadow::setLod(const qreal lod)
    {
        sLod = lod;
    }

    void NodeDragShadow::setVisualCue(const DragCue cue)
    {
        mCue = cue;
        update();
    }

    void NodeDragShadow::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        int color_index = static_cast<int>(mCue);
        assert(color_index <= 3);

        sPen.setColor(sColorPen[color_index]);
        painter->setPen(sPen);

        if (sLod < 0.5)
        {
            painter->fillRect(QRectF(0, 0, mWidth, mHeight), sColorSolid[color_index]);
        }
        else
        {
            QRectF rect = QRectF(0, 0, mWidth, mHeight);
            painter->drawRect(rect);
            painter->fillRect(rect, sColorTranslucent[color_index]);
        }
    }

    QRectF NodeDragShadow::boundingRect() const
    {
        return QRectF(0, 0, mWidth, mHeight);
    }

    QPainterPath NodeDragShadow::shape() const
    {
        QPainterPath path;
        path.addRect(QRectF(0, 0, mWidth, mHeight));
        return path;
    }
}

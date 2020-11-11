#include "gui/graph_widget/items/nets/arrow_separated_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

namespace hal
{
    qreal ArrowSeparatedNet::sWireLength;
    qreal ArrowSeparatedNet::sInputArrowOffset;
    qreal ArrowSeparatedNet::sOutputArrowOffset;
    qreal ArrowSeparatedNet::sArrowLeftXShift;
    qreal ArrowSeparatedNet::sArrowRightXShift;
    qreal ArrowSeparatedNet::sArrowSideLength;
    qreal ArrowSeparatedNet::sArrowWidth;
    qreal ArrowSeparatedNet::sArrowHeight;

    qreal ArrowSeparatedNet::sInputWidth;
    qreal ArrowSeparatedNet::sOutputWidth;

    QPainterPath ArrowSeparatedNet::sArrow;

    void ArrowSeparatedNet::loadSettings()
    {
        sWireLength = 26;

        sInputArrowOffset = 3;
        sOutputArrowOffset = 3;

        sArrowLeftXShift = 0;
        sArrowRightXShift = 3;
        sArrowSideLength = 12;

        sArrowHeight = 6;
        sArrowWidth = sArrowLeftXShift + sArrowSideLength + sArrowRightXShift;

        sInputWidth = sWireLength + sInputArrowOffset + sArrowWidth + sShapeWidth;
        sOutputWidth = sWireLength + sOutputArrowOffset + sArrowWidth + sShapeWidth;

        QPointF point(sArrowLeftXShift, -sArrowHeight / 2);

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
            // only available in Qt >= 5.13.0
            sArrow.clear();
        #else
            // low-performance fallback for older Qt
            sArrow = QPainterPath();
        #endif
        sArrow.lineTo(point);
        point.setX(point.x() + sArrowSideLength);
        sArrow.lineTo(point);
        point.setX(point.x() + sArrowRightXShift);
        point.setY(0);
        sArrow.lineTo(point);
        point.setX(point.x() - sArrowRightXShift);
        point.setY(sArrowHeight / 2);
        sArrow.lineTo(point);
        point.setX(point.x() - sArrowSideLength);
        sArrow.lineTo(point);
        sArrow.closeSubpath();
    }

    ArrowSeparatedNet::ArrowSeparatedNet(Net* n) : SeparatedGraphicsNet(n)
    {
    }

    void ArrowSeparatedNet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        if (sLod < graph_widget_constants::sEparatedNetMinLod)
            return;

        QColor color = penColor(option->state);
        color.setAlphaF(sAlpha);

        sPen.setColor(color);
        painter->setPen(sPen);

        if (mFillIcon)
        {
            sBrush.setColor(color);
            sBrush.setStyle(mBrushStyle);
            painter->setBrush(sBrush);
        }

        const Qt::PenStyle original_pen_style = sPen.style();

        for (const QPointF& position : mInputPositions)
        {
            QPointF to(position.x() - sWireLength, position.y());
            painter->drawLine(position, to);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            sPen.setStyle(Qt::PenStyle::SolidLine);
            painter->setPen(sPen);
            painter->translate(QPointF(position.x() - sWireLength - sInputArrowOffset - sArrowWidth, position.y()));
            painter->drawPath(sArrow);
            sPen.setStyle(original_pen_style);
            painter->restore();
        }

        for (const QPointF& position : mOutputPositions)
        {
            QPointF to(position.x() + sWireLength, position.y());
            painter->drawLine(position, to);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            sPen.setStyle(Qt::PenStyle::SolidLine);
            painter->setPen(sPen);
            painter->translate(QPointF(position.x() + sWireLength + sOutputArrowOffset, position.y()));
            painter->drawPath(sArrow);
            sPen.setStyle(original_pen_style);
            painter->restore();
        }

        sBrush.setStyle(Qt::NoBrush);
        painter->setBrush(sBrush);

    #ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
        bool original_cosmetic = sPen.isCosmetic();
        sPen.setCosmetic(true);
        sPen.setColor(Qt::green);
        painter->setPen(sPen);
        painter->drawPath(mShape);
        sPen.setCosmetic(original_cosmetic);
    #endif
    }

    void ArrowSeparatedNet::addInput(const QPointF& scene_position)
    {
        QPointF mapped_position = mapFromScene(scene_position);
        mInputPositions.append(mapped_position);

        const qreal half_of_shape_width = sShapeWidth / 2;

        QPointF point(mapped_position.x() - sWireLength - half_of_shape_width, mapped_position.y() - half_of_shape_width);

        mShape.moveTo(point);
        point.setX(point.x() + sWireLength + sShapeWidth);
        mShape.lineTo(point);
        point.setY(point.y() + sShapeWidth);
        mShape.lineTo(point);
        point.setX(point.x() - sWireLength - sShapeWidth);
        mShape.lineTo(point);
        mShape.closeSubpath();

        point.setX(mapped_position.x() - sWireLength - sInputArrowOffset - sArrowWidth - half_of_shape_width);
        point.setY(mapped_position.y() - sArrowHeight / 2 - half_of_shape_width);

        mShape.moveTo(point);
        point.setX(point.x() + sArrowWidth + sShapeWidth);
        mShape.lineTo(point);
        point.setY(point.y() + sArrowHeight + sShapeWidth);
        mShape.lineTo(point);
        point.setX(point.x() - sArrowWidth - sShapeWidth);
        mShape.lineTo(point);
        mShape.closeSubpath();
    }

    void ArrowSeparatedNet::addOutput(const QPointF& scene_position)
    {
        QPointF mapped_position = mapFromScene(scene_position);
        mOutputPositions.append(mapped_position);

        const qreal half_of_shape_width = sShapeWidth / 2;

        QPointF point(mapped_position.x() - half_of_shape_width, mapped_position.y() - half_of_shape_width);

        mShape.moveTo(point);
        point.setX(point.x() + sWireLength + sShapeWidth);
        mShape.lineTo(point);
        point.setY(point.y() + sShapeWidth);
        mShape.lineTo(point);
        point.setX(point.x() - sWireLength - sShapeWidth);
        mShape.lineTo(point);
        mShape.closeSubpath();

        point.setX(mapped_position.x() + sWireLength + sOutputArrowOffset - half_of_shape_width);
        point.setY(mapped_position.y() - sArrowHeight / 2 - half_of_shape_width);

        mShape.moveTo(point);
        point.setX(point.x() + sArrowWidth + sShapeWidth);
        mShape.lineTo(point);
        point.setY(point.y() + sArrowHeight + sShapeWidth);
        mShape.lineTo(point);
        point.setX(point.x() - sArrowWidth - sShapeWidth);
        mShape.lineTo(point);
        mShape.closeSubpath();
    }

    qreal ArrowSeparatedNet::inputWidth() const
    {
        return sInputWidth;
    }

    qreal ArrowSeparatedNet::outputWidth() const
    {
        return sOutputWidth;
    }
}

#include "gui/graph_widget/items/nets/circle_separated_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

namespace hal
{
    qreal CircleSeparatedNet::sWireLength;
    qreal CircleSeparatedNet::sCircleOffset;
    qreal CircleSeparatedNet::sRadius;

    void CircleSeparatedNet::loadSettings()
    {
        sWireLength   = 26;
        sCircleOffset = 0;
        sRadius        = 3;
    }

    CircleSeparatedNet::CircleSeparatedNet(Net* n) : SeparatedGraphicsNet(n)
    {
    }

    void CircleSeparatedNet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

        const bool original_antialiasing = painter->renderHints() & QPainter::Antialiasing;
        const Qt::PenStyle original_pen_style = sPen.style();

        for (const QPointF& position : mInputPositions)
        {
            QPointF to(position.x() - sWireLength, position.y());
            painter->drawLine(position, to);
            painter->setRenderHint(QPainter::Antialiasing, true);
            sPen.setStyle(Qt::PenStyle::SolidLine);
            painter->setPen(sPen);
            painter->drawEllipse(QPointF(to.x() - sCircleOffset, to.y()), sRadius, sRadius);
            sPen.setStyle(original_pen_style);
            painter->setPen(sPen);
            painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
        }

        for (const QPointF& position : mOutputPositions)
        {
            QPointF to(position.x() + sWireLength, position.y());
            painter->drawLine(position, to);
            painter->setRenderHint(QPainter::Antialiasing, true);
            sPen.setStyle(Qt::PenStyle::SolidLine);
            painter->setPen(sPen);
            painter->drawEllipse(QPointF(to.x() + sWireLength + sCircleOffset, to.y()), sRadius, sRadius);
            sPen.setStyle(original_pen_style);
            painter->setPen(sPen);
            painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
        }

        sBrush.setStyle(Qt::NoBrush);
        painter->setBrush(sBrush);

    #ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
        bool original_cosmetic = sPen.isCosmetic();
        sPen.setCosmetic(true);
        sPen.setColor(Qt::green);
        painter->setPen(sPen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawPath(mShape);
        painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
        sPen.setCosmetic(original_cosmetic);
    #endif
    }

    void CircleSeparatedNet::addInput(const QPointF& scene_position)
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

        point.setX(mapped_position.x() - sWireLength - sCircleOffset);
        point.setY(mapped_position.y());
        const qreal radius = sRadius + half_of_shape_width;

        mShape.addEllipse(point, radius, radius);
    }

    void CircleSeparatedNet::addOutput(const QPointF& scene_position)
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

        point.setX(mapped_position.x() + sWireLength + sCircleOffset);
        point.setY(mapped_position.y());
        const qreal radius = sRadius + half_of_shape_width;

        mShape.addEllipse(point, radius, radius);
    }

    qreal CircleSeparatedNet::inputWidth() const
    {
        return sWireLength + sCircleOffset + sRadius + sLineWidth / 2;
    }

    qreal CircleSeparatedNet::outputWidth() const
    {
        return sWireLength + sCircleOffset + sRadius + sLineWidth / 2;
    }
}

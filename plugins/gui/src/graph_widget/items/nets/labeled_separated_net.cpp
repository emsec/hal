#include "gui/graph_widget/items/nets/labeled_separated_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

namespace hal
{
    static const qreal sBaseline = 1;

    qreal LabeledSeparatedNet::sWireLength;
    qreal LabeledSeparatedNet::sTextOffset;

    QFont LabeledSeparatedNet::sFont;
    qreal LabeledSeparatedNet::sFontHeight;
    qreal LabeledSeparatedNet::sFontAscend;

    void LabeledSeparatedNet::loadSettings()
    {
        sWireLength = 20;
        sTextOffset = 2.4;

        sFont = QFont("Iosevka");
        sFont.setPixelSize(12);
        QFontMetricsF fm(sFont);
        sFontHeight = fm.height();
        sFontAscend = fm.ascent();
    }

    LabeledSeparatedNet::LabeledSeparatedNet(Net* n, const QString& text) : SeparatedGraphicsNet(n),
      mText(text)
    {
        QFontMetricsF fm(sFont);
        mTextWidth = fm.width(mText);
    }

    void LabeledSeparatedNet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        if (sLod < graph_widget_constants::sEparatedNetMinLod)
            return;

        QColor color = penColor(option->state);
        color.setAlphaF(sAlpha);

        sPen.setColor(color);
        painter->setPen(sPen);
        painter->setFont(sFont);

        if (mFillIcon)
        {
            sBrush.setColor(mFillColor);
            sBrush.setStyle(mBrushStyle);
            painter->setBrush(sBrush); // ???
        }

        for (const QPointF& position : mInputPositions)
        {
            if (mFillIcon)
                painter->fillRect(QRectF(position.x() - sWireLength - sTextOffset - mTextWidth, position.y() - sFontHeight / 2, mTextWidth, sFontHeight), sBrush);

            QPointF point(position.x() - sWireLength, position.y());
            painter->drawLine(position, point);
            point.setX(point.x() - sTextOffset - mTextWidth);
            point.setY(point.y() + sBaseline + sFontAscend - sFontHeight / 2);
            painter->drawText(point, mText);
        }

        for (const QPointF& position : mOutputPositions)
        {
            if (mFillIcon)
                painter->fillRect(QRectF(position.x() + sWireLength + sTextOffset, position.y() - sFontHeight / 2, mTextWidth, sFontHeight), sBrush);

            QPointF point(position.x() + sWireLength, position.y());
            painter->drawLine(position, point);
            point.setX(point.x() + sTextOffset);
            point.setY(point.y() + sBaseline + sFontAscend - sFontHeight / 2);
            painter->drawText(point, mText);
        }

        sBrush.setStyle(Qt::NoBrush);
        painter->setBrush(sBrush);

    #ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
        sPen.setColor(Qt::green);
        painter->setPen(sPen);
        painter->drawPath(mShape);
    #endif
    }

    void LabeledSeparatedNet::addInput(const QPointF& scene_position)
    {
        const QPointF mapped_position = mapFromScene(scene_position);
        mInputPositions.append(mapped_position);

        const qreal half_of_shape_width = sShapeWidth / 2;
        const qreal half_of_font_height = sFontHeight / 2;

        QPointF point(mapped_position.x() - sWireLength - half_of_shape_width, mapped_position.y() - half_of_shape_width);

        mShape.moveTo(point);
        point.setX(point.x() + sWireLength + sShapeWidth);
        mShape.lineTo(point);
        point.setY(point.y() + sShapeWidth);
        mShape.lineTo(point);
        point.setX(point.x() - sWireLength - sShapeWidth);
        mShape.lineTo(point);
        mShape.closeSubpath();

        point.setX(mapped_position.x() - sWireLength - sTextOffset - mTextWidth); // - half_of_shape_width
        point.setY(mapped_position.y() - half_of_font_height); // - half_of_shape_width

        mShape.moveTo(point);
        point.setX(point.x() + mTextWidth); // + sShapeWidth
        mShape.lineTo(point);
        point.setY(point.y() + sFontHeight); // + sShapeWidth
        mShape.lineTo(point);
        point.setX(point.x() - mTextWidth); // - sShapeWidth
        mShape.lineTo(point);
        mShape.closeSubpath();
    }

    void LabeledSeparatedNet::addOutput(const QPointF& scene_position)
    {
        const QPointF mapped_position = mapFromScene(scene_position);
        mOutputPositions.append(mapped_position);

        const qreal half_of_shape_width = sShapeWidth / 2;
        const qreal half_of_font_height = sFontHeight / 2;

        QPointF point(mapped_position.x() - half_of_shape_width, mapped_position.y() - half_of_shape_width);

        mShape.moveTo(point);
        point.setX(point.x() + sWireLength + sShapeWidth);
        mShape.lineTo(point);
        point.setY(point.y() + sShapeWidth);
        mShape.lineTo(point);
        point.setX(point.x() - sWireLength - sShapeWidth);
        mShape.lineTo(point);
        mShape.closeSubpath();

        point.setX(mapped_position.x() + sWireLength + sTextOffset); // - half_of_shape_width
        point.setY(mapped_position.y() - half_of_font_height); // - half_of_shape_width

        mShape.moveTo(point);
        point.setX(point.x() + mTextWidth); // + sShapeWidth
        mShape.lineTo(point);
        point.setY(point.y() + sFontHeight); // + sShapeWidth
        mShape.lineTo(point);
        point.setX(point.x() - mTextWidth); // - sShapeWidth
        mShape.lineTo(point);
        mShape.closeSubpath();
    }

    qreal LabeledSeparatedNet::inputWidth() const
    {
        return sWireLength + sTextOffset + mTextWidth;
    }

    qreal LabeledSeparatedNet::outputWidth() const
    {
        return sWireLength + sTextOffset + mTextWidth;
    }
}

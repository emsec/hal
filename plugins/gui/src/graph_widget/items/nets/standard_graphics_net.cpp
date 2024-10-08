#include "gui/graph_widget/items/nets/standard_graphics_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>
#include <assert.h>
#include <limits>
#include <QThread>
#include <QDebug>
#include <QGraphicsScene>

namespace hal
{
    qreal StandardGraphicsNet::sAlpha;

    qreal StandardGraphicsNet::sWireLength;
    qreal StandardGraphicsNet::sLeftArrowOffset;
    qreal StandardGraphicsNet::sRightArrowOffset;
    qreal StandardGraphicsNet::sArrowLeftXShift;
    qreal StandardGraphicsNet::sArrowRightXShift;
    qreal StandardGraphicsNet::sArrowSideLength;
    qreal StandardGraphicsNet::sArrowWidth;
    qreal StandardGraphicsNet::sArrowHeight;

    QPainterPath StandardGraphicsNet::sArrow;

    qreal StandardGraphicsNet::sSplitRadius;

    void StandardGraphicsNet::loadSettings()
    {
        sWireLength = 26;

        sLeftArrowOffset  = 3;
        sRightArrowOffset = 3;

        sArrowLeftXShift  = 0;
        sArrowRightXShift = 3;
        sArrowSideLength   = 12;

        sArrowHeight = 6;
        sArrowWidth  = sArrowLeftXShift + sArrowSideLength + sArrowRightXShift;

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

        sSplitRadius = 3;
    }

    void StandardGraphicsNet::updateAlpha()
    {
        if (sLod >= graph_widget_constants::sNetFadeInLod && sLod <= graph_widget_constants::sNetFadeOutLod)
            sAlpha = (sLod - graph_widget_constants::sNetFadeInLod) / (graph_widget_constants::sNetFadeOutLod - graph_widget_constants::sNetFadeInLod);
        else
            sAlpha = 1;
    }

    StandardGraphicsNet::StandardGraphicsNet(Net* n, const Lines& l, const QList<QPointF> &knots) : GraphicsNet(n)
    {
        for (const QPointF& point : knots)
        {
            mSplits.append(point);
            mShape.addEllipse(point, sSplitRadius, sSplitRadius);
        }

        qreal smallest_x = std::numeric_limits<qreal>::max();
        qreal biggest_x  = std::numeric_limits<qreal>::min();

        qreal smallest_y = std::numeric_limits<qreal>::max();
        qreal biggest_y  = std::numeric_limits<qreal>::min();

        for (const HLine& h : l.mHLines)
        {
            if (h.mSmallX < smallest_x)
                smallest_x = h.mSmallX;

            if (h.mBigX > biggest_x)
                biggest_x = h.mBigX;

            if (h.y < smallest_y)
                smallest_y = h.y;
            if (h.y > biggest_y)
                biggest_y = h.y;

            QLineF line(h.mSmallX, h.y, h.mBigX, h.y);
            mLines.append(line);
            QRectF rect(h.mSmallX - sShapeWidth / 2, h.y - sShapeWidth / 2, h.mBigX - h.mSmallX + sShapeWidth, sShapeWidth);
            mShape.addRect(rect);
        }

        for (const VLine& v : l.mVLines)
        {
            if (v.x < smallest_x)
                smallest_x = v.x;
            else if (v.x > biggest_x)
                biggest_x = v.x;

            if (v.mSmallY < smallest_y)
                smallest_y = v.mSmallY;

            if (v.mBigY > biggest_y)
                biggest_y = v.mBigY;

            QLineF line(v.x, v.mSmallY, v.x, v.mBigY);
            mLines.append(line);
            QRectF rect(v.x - sShapeWidth / 2, v.mSmallY - sShapeWidth / 2, sShapeWidth, v.mBigY - v.mSmallY + sShapeWidth);
            mShape.addRect(rect);
        }

        const qreal padding = sSplitRadius + sShapeWidth;

        mRect = QRectF(smallest_x - padding, smallest_y - padding, biggest_x - smallest_x + padding, biggest_y - smallest_y + padding);
    }

    void StandardGraphicsNet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        QColor color = penColor(option->state);
        sPen.setColor(color);
        sPen.setStyle(mPenStyle);

        painter->setPen(sPen);
        painter->drawLines(mLines);

        if (sLod > graph_widget_constants::sNetFadeInLod)
        {
            color.setAlphaF(sAlpha);

            sPen.setColor(color);
            sPen.setStyle(Qt::SolidLine);
            painter->setPen(sPen);

            sBrush.setColor(color);
            sBrush.setStyle(Qt::SolidPattern);
            painter->setBrush(sBrush);

            const bool original_antialiasing = painter->renderHints().testFlag(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::Antialiasing, true);

            for (const QPointF& point : mSplits)
                painter->drawEllipse(point, sSplitRadius, sSplitRadius);

            painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
        }

        sBrush.setStyle(Qt::NoBrush);
        painter->setBrush(sBrush);

#ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
        sPen.setColor(Qt::green);
        const bool original_cosmetic = sPen.isCosmetic();
        sPen.setCosmetic(true);
        painter->setPen(sPen);
        painter->drawPath(mShape);
        sPen.setCosmetic(original_cosmetic);
#endif
    }

    void StandardGraphicsNet::Lines::appendHLine(const qreal mSmallX, const qreal mBigX, const qreal y)
    {
        assert(mSmallX < mBigX);

        mHLines.append(HLine{mSmallX, mBigX, y});
    }

    void StandardGraphicsNet::Lines::appendVLine(const qreal x, const qreal mSmallY, const qreal mBigY)
    {
        assert(mSmallY < mBigY);

        mVLines.append(VLine{x, mSmallY, mBigY});
    }
}    // namespace hal

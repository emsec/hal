#include "gui/graph_widget/items/nets/old_standard_graphics_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

namespace hal
{
    qreal OldStandardGraphicsNet::sAlpha;

    qreal OldStandardGraphicsNet::sWireLength;
    qreal OldStandardGraphicsNet::sLeftArrowOffset;
    qreal OldStandardGraphicsNet::sRightArrowOffset;
    qreal OldStandardGraphicsNet::sArrowLeftXShift;
    qreal OldStandardGraphicsNet::sArrowRightXShift;
    qreal OldStandardGraphicsNet::sArrowSideLength;
    qreal OldStandardGraphicsNet::sArrowWidth;
    qreal OldStandardGraphicsNet::sArrowHeight;

    QPainterPath OldStandardGraphicsNet::sArrow;

    qreal OldStandardGraphicsNet::sSplitRadius;

    void OldStandardGraphicsNet::loadSettings()
    {
        sWireLength = 26;

        sLeftArrowOffset = 3;
        sRightArrowOffset = 3;

        sArrowLeftXShift = 0;
        sArrowRightXShift = 3;
        sArrowSideLength = 12;

        sArrowHeight = 6;
        sArrowWidth = sArrowLeftXShift + sArrowSideLength + sArrowRightXShift;

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

    void OldStandardGraphicsNet::updateAlpha()
    {
        if (sLod >= graph_widget_constants::sNetFadeInLod && sLod <= graph_widget_constants::sNetFadeOutLod)
            sAlpha = (sLod - graph_widget_constants::sNetFadeInLod) / (graph_widget_constants::sNetFadeOutLod - graph_widget_constants::sNetFadeInLod);
        else
            sAlpha = 1;
    }

    //StandardGraphicsNet::StandardGraphicsNet(Net* n, const lines& l) : GraphicsNet(n),
    OldStandardGraphicsNet::OldStandardGraphicsNet(Net* n, Lines& l, const bool complete) : GraphicsNet(n),
        mComplete(complete)
    {
        QVector<HLine> collapsed_h;
        QVector<VLine> collapsed_v;

        //for (const HLine& h : l.mHLines)
        for (HLine& h : l.mHLines)
        {
            if (h.mSmallX == h.mBigX)
                continue;

            //assert(h.mSmallX < h.mBigX);
            if (h.mSmallX > h.mBigX)
            {
                qreal temp = h.mSmallX;
                h.mSmallX = h.mBigX;
                h.mBigX = temp;
            }

            QVector<int> overlaps;

            for (int i = 0; i < collapsed_h.size(); ++i)
                if (h.y == collapsed_h.at(i).y)
                {
                    if (h.mBigX < collapsed_h.at(i).mSmallX || collapsed_h.at(i).mBigX < h.mSmallX)
                        continue; // NO OVERLAP

                    // OVERLAP
                    overlaps.append(i);
                }


            if (overlaps.isEmpty())
                collapsed_h.append(h);
            else
            {
                qreal smallest_x = h.mSmallX;
                qreal biggest_x = h.mBigX;

                for (int i = 0; i < overlaps.size(); ++i)
                {
                    int index = overlaps.at(i) - i;

                    if (collapsed_h.at(index).mSmallX < smallest_x)
                        smallest_x = collapsed_h.at(index).mSmallX;

                    if (collapsed_h.at(index).mBigX > biggest_x)
                        biggest_x = collapsed_h.at(index).mBigX;

                    collapsed_h.remove(index);
                }

                collapsed_h.append(HLine{smallest_x, biggest_x, h.y});
            }
        }

        //for (const VLine& v : l.mVLines)
        for (VLine& v : l.mVLines)
        {
            if (v.mSmallY == v.mBigY)
                continue;

            //assert(v.mSmallY < v.mBigY);
            if (v.mSmallY > v.mBigY)
            {
                qreal temp = v.mSmallY;
                v.mSmallY = v.mBigY;
                v.mBigY = temp;
            }

            QVector<int> overlaps;

            for (int i = 0; i < collapsed_v.size(); ++i)
                if (v.x == collapsed_v.at(i).x)
                {
                    if (v.mBigY < collapsed_v.at(i).mSmallY || collapsed_v.at(i).mBigY < v.mSmallY)
                        continue; // NO OVERLAP

                    // OVERLAP
                    overlaps.append(i);
                }


            if (overlaps.isEmpty())
                collapsed_v.append(v);
            else
            {
                qreal smallest_y = v.mSmallY;
                qreal biggest_y = v.mBigY;

                for (int i = 0; i < overlaps.size(); ++i)
                {
                    int index = overlaps.at(i) - i;

                    if (collapsed_v.at(index).mSmallY < smallest_y)
                        smallest_y = collapsed_v.at(index).mSmallY;

                    if (collapsed_v.at(index).mBigY > biggest_y)
                        biggest_y = collapsed_v.at(index).mBigY;

                    collapsed_v.remove(index);
                }

                collapsed_v.append(VLine{v.x, smallest_y, biggest_y});
            }
        }

        // CALCULATE SPLITS
        for (const HLine& h : collapsed_h)
        {
            for (const VLine& v : collapsed_v)
            {
                // ON EVERY INTERSECTION
    //            if (h.mSmallX <= v.x && v.x <= h.mBigX)
    //                if (v.mSmallY <= h.y && h.y <= v.mBigY)
    //                {
    //                    QPointF point(v.x - l.src_x, h.y - l.src_y);
    //                    mSplits.append(point);
    //                    mShape.addEllipse(point, sRadius, sRadius);
    //                }

                // ONLY ON REAL SPLITS
                if (h.mSmallX <= v.x && v.x <= h.mBigX)
                    if (v.mSmallY < h.y && h.y < v.mBigY)
                    {
                        QPointF point(v.x, h.y);
                        mSplits.append(point);
                        mShape.addEllipse(point, sSplitRadius, sSplitRadius);
                    }

                if (v.mSmallY <= h.y && h.y <= v.mBigY)
                    if (h.mSmallX < v.x && v.x < h.mBigX)
                    {
                        QPointF point(v.x, h.y);
                        mSplits.append(point);
                        mShape.addEllipse(point, sSplitRadius, sSplitRadius);
                    }
            }
        }

        // CALCULATE RECT AND SHAPE
        // 0 INITIALIZATION ONLY WORKS WHEN NETS ARE PLACED AT THE SRC POSITION, CHANGE ?
        qreal smallest_x = 0;
        qreal biggest_x = 0;
        qreal smallest_y = 0;
        qreal biggest_y = 0;

        for (const HLine& h : collapsed_h)
        {
            qreal mSmallX = h.mSmallX;
            qreal mBigX = h.mBigX;
            qreal y = h.y;

            if (mSmallX < smallest_x)
                smallest_x = mSmallX;

            if (mBigX > biggest_x)
                biggest_x = mBigX;

            if (y < smallest_y)
                smallest_y = y;
            else if (y > biggest_y)
                biggest_y = y;

            QLineF line(mSmallX, y, mBigX, y);
            mOtherLines.append(line);
            QRectF rect(mSmallX - sShapeWidth / 2, y - sShapeWidth / 2, mBigX - mSmallX + sLineWidth + sShapeWidth, sLineWidth + sShapeWidth);
            mShape.addRect(rect);
        }

        for (const VLine& v : collapsed_v)
        {
            qreal x = v.x;
            qreal mSmallY = v.mSmallY;
            qreal mBigY = v.mBigY;

            if (x < smallest_x)
                smallest_x = x;
            else if (x > biggest_x)
                biggest_x = x;

            if (mSmallY < smallest_y)
                smallest_y = mSmallY;

            if (mBigY > biggest_y)
                biggest_y = mBigY;

            QLineF line(x, mSmallY, x, mBigY);
            mOtherLines.append(line);
            QRectF rect(x - sShapeWidth / 2, mSmallY - sShapeWidth / 2, sLineWidth + sShapeWidth, mBigY - mSmallY + sLineWidth + sShapeWidth);
            mShape.addRect(rect);
        }

        // COMPENSATE FOR PEN WIDTH ?
        // COMPENSATE FOR SPLITS ?
        mRect = QRectF(smallest_x, smallest_y, biggest_x - smallest_x, biggest_y - smallest_y);

    //    for (const HLine& h : l.mHLines)
    //        mLines.append(QLineF(h.mSmallX - l.src_x, h.y - l.src_y, h.mBigX - l.src_x, h.y - l.src_y));

    //    for (const VLine& v : l.mVLines)
        //        mLines.append(QLineF(v.x - l.src_x, v.mSmallY - l.src_y, v.x - l.src_x, v.mBigY - l.src_y));
    }

    void OldStandardGraphicsNet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        QColor color = penColor(option->state);
        sPen.setColor(color);
        sPen.setStyle(mPenStyle);

        painter->setPen(sPen);
        painter->drawLines(mOtherLines);

        if (sLod > graph_widget_constants::sNetFadeInLod)
        {
            color.setAlphaF(sAlpha);

            sPen.setColor(color);
            painter->setPen(sPen);

            sBrush.setColor(color);

            const bool original_antialiasing = painter->renderHints().testFlag(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::Antialiasing, true);

            if (!mComplete)
            {
                if (mFillIcon)
                {
                    sBrush.setStyle(mBrushStyle);
                    painter->setBrush(sBrush);
                }

                QPointF translation_value(sWireLength + sLeftArrowOffset, 0);
                painter->translate(translation_value);
                painter->drawPath(sArrow);
                painter->translate(-translation_value);
            }

            sBrush.setStyle(Qt::SolidPattern);
            painter->setBrush(sBrush);

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
}

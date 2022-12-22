#include "gui/comment_system/comment_speech_bubble.h"
#include "gui/graph_widget/contexts/graph_context.h"

#include <QRectF>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDebug>

namespace hal {
    CommentSpeechBubble::CommentSpeechBubble(const QString &txt, const Node &nd, GraphContext *ctx)
        : mHover(false), mBubble(QRectF(-50,-15,sBubbleWidth,35)), mText(txt), mNode(nd), mContext(ctx)
    {
        setAcceptHoverEvents(true);
    }

    QRectF CommentSpeechBubble::boundingRect() const
    {
        QRectF bb(mBubble);
        bb.setRight(bb.right()+sBubbleWidthExtension);
        return bb;
    }

    QPainterPath CommentSpeechBubble::shape() const
    {
        QPainterPath retval;
        retval.addRect(mBubble);
        return retval;
    }

    void CommentSpeechBubble::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        Q_UNUSED(event);
        mHover = true;
        update();
    }

    void CommentSpeechBubble::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        Q_UNUSED(event);
        mHover = false;
        update();
    }

    void CommentSpeechBubble::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        Q_UNUSED(event);
        if (!mContext) return;
        mContext->showComments(mNode);
    }

    void CommentSpeechBubble::paintText(QPainter *painter, int maxWidth)
    {
        QFontMetrics fm(painter->font());
        QString txt = mText;
        int n = mText.size();
        while ((fm.horizontalAdvance(txt) + 2 * sTextMargin) > maxWidth && n > 1)
        {
            --n;
            txt = mText.left(n) + "…";
        }
        QRectF rText = mBubble;
        rText.setWidth(maxWidth);
        rText.setLeft(rText.left()+sTextMargin);
        rText.setRight(rText.right()-sTextMargin);
        rText.setHeight(sTextHeight);
        painter->drawText(rText,Qt::AlignLeft|Qt::AlignVCenter,txt);
    }

    void CommentSpeechBubble::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->setPen(QPen(Qt::black,0)); // border color
        QRectF rBubble=mBubble;
        rBubble.setHeight(sTextHeight);
        int txtWidth = rBubble.width();
        QColor bgColor("#F0E0C0"); // background color
        if (mHover)
        {
            QFontMetrics fm(painter->font());
            int w = fm.horizontalAdvance(mText);
            txtWidth = w+8>boundingRect().width() ? boundingRect().width() : w+8;
            rBubble.setWidth(txtWidth);
        }
        else
        {
            bgColor.setAlpha(80);
        }
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rBubble,8,4);
        QVector<QPointF> pts;
        pts.append(QPointF(-50,20));
        pts.append(QPointF(-30,11));
        pts.append(QPointF(-10,12));
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(QPolygonF(pts));
        pts.append(QPointF(-50,20));
        painter->setPen(QPen(Qt::black,0)); // text color
        painter->drawLines(pts);
        paintText(painter,txtWidth);
    }

}

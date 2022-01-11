#include "vcd_viewer/wave_cursor.h"
#include "vcd_viewer/wave_scene.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QDebug>

namespace hal {

    WaveCursorLabel::WaveCursorLabel(const QRectF& bbox, QGraphicsItem *parentItem)
        : QGraphicsItem(parentItem), mBoundingRect(bbox)
    {
        setFlags(flags() | QGraphicsItem::ItemIgnoresTransformations);
    }

    void WaveCursorLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        const WaveCursor* wc = static_cast<const WaveCursor*>(parentItem());
        float xpos = wc ? wc->pos().x() : 0;
        painter->setBrush(Qt::black);
        painter->setPen(QPen(QBrush(Qt::red),0));
        painter->drawRoundedRect(mBoundingRect, 45, 75, Qt::RelativeSize);
        painter->setPen(QPen(QColor("#c0c0c0")));
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSizeF(10);
        painter->setFont(font);
        painter->drawText(mBoundingRect, QString::number(xpos,'f',0), Qt::AlignHCenter | Qt::AlignVCenter);

    }

    QRectF WaveCursorLabel::boundingRect() const
    {
        return mBoundingRect;
    }

//===================================================00000
    WaveCursor::WaveCursor()
        : mXmag(1)
    {
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemSendsGeometryChanges);
        float w = boundingRect().width();
        new WaveCursorLabel(QRectF(-w/2,-25,w,30),this);
    }

    QRectF WaveCursor::boundingRect() const
    {
        float dx = mXmag > 0 ? 40 / mXmag : 4;
        return QRectF(-dx,-99999,2*dx,2*99999);
    }


    QPainterPath WaveCursor::shape() const
    {
        QPainterPath retval;
        retval.addRect(boundingRect());
        return retval;
    }

    void WaveCursor::xScaleChanged(float mag)
    {
        if (mXmag==mag) return;
        mXmag = mag;
    }

    QVariant WaveCursor::itemChange(GraphicsItemChange change, const QVariant &value)
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        if (sc && change == ItemPositionChange) {
               sc->emitCursorMoved(value.toPointF().x());
               update();
        }
        return QGraphicsItem::itemChange(change, value);
    }

    void WaveCursor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(QPen(QBrush(Qt::red),0));
        painter->drawLine(0,-99999,0,99999);

        /*
        QRectF rValue(boundingRect().left(),0,boundingRect().width(),2);
        QBrush storeBrush = painter->brush();
        painter->setBrush(Qt::black);
        painter->drawRoundedRect(rValue, 55, 55, Qt::RelativeSize);
        painter->setBrush(storeBrush);

        painter->setPen(QPen(QColor("#c0c0c0")));
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSizeF(1.1);
        painter->setFont(font);
        QTransform trans = painter->transform();
        painter->setTransform(QTransform(15./mXmag,0,0,1.,0,0),true);
        painter->drawText(rValue, QString::number(pos().x(),'f',1), Qt::AlignHCenter | Qt::AlignVCenter);
        painter->setTransform(trans);
*/
    }
}

#include "vcd_viewer/wave_cursor.h"
#include "vcd_viewer/wave_scene.h"
#include <QBrush>
#include <QPen>
#include <QPainter>

namespace hal {

    WaveCursor::WaveCursor()
        : mXmag(1)
    {
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemSendsGeometryChanges);
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

        WaveScene* sc = static_cast<WaveScene*>(scene());
        if (!sc) return;
        float xpos = sc->cursorPos();
        float ypos = sc->yPosition(-1);
        QRectF rValue(boundingRect().left(),ypos,boundingRect().width(),2);
        painter->fillRect(rValue, Qt::black);
        painter->drawRoundRect(rValue, 35, 35);

        painter->setPen(QPen(QColor("#c0c0c0")));
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSizeF(1.1);
        painter->setFont(font);
        QTransform trans = painter->transform();
        painter->setTransform(QTransform(15./mXmag,0,0,1.,0,0),true);
        painter->drawText(rValue, QString::number(xpos,'f',1), Qt::AlignHCenter | Qt::AlignVCenter);
        painter->setTransform(trans);

    }
}

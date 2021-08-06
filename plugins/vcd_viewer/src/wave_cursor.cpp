#include "vcd_viewer/wave_cursor.h"
#include "vcd_viewer/wave_scene.h"
#include <QBrush>
#include <QPen>
namespace hal {

    WaveCursor::WaveCursor()
        : QGraphicsLineItem(0,-99999,0,99999), mXmag(1)
    {
        setPen(QPen(QBrush(Qt::red),0));
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
}

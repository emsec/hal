#include "dot_viewer/dot_graphics_view.h"
#include <qmath.h>

namespace hal {
    DotGraphicsView::DotGraphicsView(QWidget* parent)
        : QGraphicsView(parent)
    {;}

    void DotGraphicsView::wheelEvent(QWheelEvent* event)
    {
        qreal scaleFactor = qPow(2.0, event->delta() / 240.0); //How fast we zoom
        scaleWithinLimits(scaleFactor);

    }

    void DotGraphicsView::handleZoomInShortcut()
    {
        scaleWithinLimits(1.2);
    }

    void DotGraphicsView::handleZoomOutShortcut()
    {
        scaleWithinLimits(0.8);
    }

    void DotGraphicsView::scaleWithinLimits(qreal scaleFactor)
    {
        qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
        if(0.1 < factor && factor < 100) scale(scaleFactor, scaleFactor);
    }
}

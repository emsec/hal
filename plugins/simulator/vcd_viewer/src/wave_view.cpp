#include "vcd_viewer/wave_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_label.h"
#include "netlist_simulator_controller/wave_data.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QScrollBar>

namespace hal {

    WaveView::WaveView(QWidget *parent)
        : QGraphicsView(parent),
          mXmag(1), mYmag(1), mDx(0), mDy(0), mLastCursorPos(0), mLastWidth(0), mCursorPixelPos(20)
    {;}

    void WaveView::resizeEvent(QResizeEvent *event)
    {
        float scHeight = sceneRect().height();
        if (scHeight <= 0) return;
        float scWidth = sceneRect().width();
        if (scWidth <= 0) return;

        int viewHeight = event ? event->size().height() : height();
        int viewWidth  = event ? event->size().width()  : width();

        mYmag = viewHeight / scHeight;
        mDy = - mYmag* sceneRect().top();

        if (abs (mLastWidth - viewWidth) > 20)
        {
            mLastWidth = viewWidth;

            mXmag = mYmag;

            mDx = 0;

            mXmagMin = mLastWidth * 0.95 / scWidth;
        }

        setTransform(QTransform(mXmag, 0, 0, mYmag, mDx, mDy),false);
        Q_EMIT changedXscale(mXmag);

        WaveScene* sc = static_cast<WaveScene*>(scene());
        float xw = width()/mXmag;
        ensureVisible(QRectF(0,sceneRect().top(),xw,sceneRect().height()));
        sc->setCursorPos(xw/10,false);
    }

    void WaveView::scrollContentsBy(int dx, int dy)
    {
        QGraphicsView::scrollContentsBy(dx,dy);
        if (dx) restoreCursor();
        Q_EMIT relativeYScroll(dy);
    }

    void WaveView::wheelEvent(QWheelEvent *event)
    {
        int deltaY = event->angleDelta().y();
        if (abs(deltaY) < 2) return;

        float scaleFac = event->angleDelta().y() > 0 ? 1.2 : 1./1.2;
        float m = mXmag * scaleFac;
        if (m < mXmagMin) m = mXmagMin;
        if (m > 500) m = 500;
        if (m == mXmag) return;
        scaleFac = m/mXmag;
        mXmag = m;

        int ixMouse = (int) floor(event->pos().x()+0.5);
        QPointF scenePoint = mapToScene(ixMouse,0);

        float xfix = scenePoint.x();
        float x0 = xfix - ixMouse / mXmag;
        if (x0 < 0) x0 = 0;
        float x1 = x0 + width() / mXmag;

        fitInView(x0,sceneRect().top(),x1-x0,sceneRect().height());

        Q_EMIT (changedXscale(transform().m11()));

        restoreCursor();
    }

    void WaveView::restoreCursor()
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        int x0 = mapFromScene(0,0).x();
        if (mCursorPixelPos < x0)
            // last cursor pixel pos not in diagram
            sc->setCursorPos(sc->cursorPos(),false);
        else
        {
            QPointF sCurs = mapToScene(mCursorPixelPos,0);
            sc->setCursorPos(sCurs.x(),false);
        }
    }

    void WaveView::handleCursorMoved(float xpos)
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        if (!sc) return;

        if (xpos == mLastCursorPos) return;
        mLastCursorPos = xpos;

        QPoint pos = mapFromScene(QPointF(xpos,0));
        int ix = pos.x();
        if (ix >= 0 && ix < width())
            mCursorPixelPos = ix;
    }
}

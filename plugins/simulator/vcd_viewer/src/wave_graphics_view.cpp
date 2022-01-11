#include "vcd_viewer/wave_graphics_view.h"
#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_timescale.h"
#include "vcd_viewer/wave_scene.h"
#include <QScrollBar>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDebug>
#include <math.h>

namespace hal {
    const float WaveGraphicsView::sYmag = 14;

    WaveGraphicsView::WaveGraphicsView(QWidget *parent)
        : QGraphicsView(parent), mXmag(1), mXmagMin(0.0001), mMinViewportHeight(1)
    {
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        mTimescale = new WaveTimescale(this);
        mTimescale->move(0,0);
    }

    void WaveGraphicsView::scrollContentsBy(int dx, int dy)
    {
        QGraphicsView::scrollContentsBy(dx,dy);
        if (dy)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(parent());
            int ypos = verticalScrollBar()->value();
            ww->scrollToYpos(ypos);
            mTimescale->move(0,0);
        }
        if (dx)
        {
            mTimescale->setScale(mXmag,scene()->sceneRect().width(),horizontalScrollBar()->value());
        }
    }

    void WaveGraphicsView::setDefaultTransform()
    {
        mXmag = 1;
        setTransform(QTransform(mXmag,0,0,sYmag,0,0));
    }

    void WaveGraphicsView::resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
//SIZE        qDebug() << "resize" << event->oldSize().height() << event->size().height() << height() << viewport()->height();
        mTimescale->setScale(mXmag,scene()->sceneRect().width(),horizontalScrollBar()->value());
    }

    void WaveGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
    {
        setCursorPos(event->pos());
    }

    void WaveGraphicsView::setCursorPos(const QPoint &pos)
    {
        QPointF scenePoint = mapToScene(pos);
        WaveScene* wc = static_cast<WaveScene*>(scene());
        wc->setCursorPosition(scenePoint);
    }

    void WaveGraphicsView::wheelEvent(QWheelEvent *event)
    {
        int deltaY = event->angleDelta().y();
        if (abs(deltaY) < 2) return;

        float scWidth = sceneRect().width() > 0 ? sceneRect().width() : 1;

        if (deltaY > 0 && mXmag > 200) return;
        if (deltaY < 0 && mXmag < 0.9 * width() / scWidth) return;
        float scaleFac = event->angleDelta().y() > 0 ? 1.2 : 1./1.2;
        float m = mXmag * scaleFac;
        if (m == mXmag) return;
        mXmag = m;

        int ixMouseViewp = (int) floor(event->pos().x()+0.25);
//        int ixMouseViewp = (int) floor(event->position().x()+0.25);
        float xMouseScene = mapToScene(ixMouseViewp,0).x();

        setTransform(QTransform(mXmag, 0, 0, 14, 0, 0),false);

        Q_EMIT (changedXscale(mXmag));


        horizontalScrollBar()->setValue(xMouseScene*mXmag -ixMouseViewp);

        mTimescale->setScale(mXmag,scene()->sceneRect().width(),horizontalScrollBar()->value());


//        restoreCursor();
    }

    void WaveGraphicsView::handleSizeChanged(int treeViewportHeight, int scrollbarMax, int scrollbarPos)
    {
//SIZE        qDebug() << "handleSizeChanged" << height() << viewport()->height() << "tree" << treeViewportHeight << "scrollbar" << verticalScrollBar()->maximum();
  /*
        if (viewport()->height() < viewportHeight)
        {
            mMinViewportHeight = viewportHeight;
            viewport()->setMinimumHeight(viewportHeight);
            qDebug() << "adjusted viewport" << viewport()->height();
        }
        */
            viewport()->setFixedHeight(treeViewportHeight + 30);
            if (scrollbarMax >= 0)
            {
                verticalScrollBar()->setMaximum(scrollbarMax);
                verticalScrollBar()->setValue(scrollbarPos);
//SIZE                qDebug() << "scrollbar" << treeViewportHeight << scrollbarMax << scrollbarPos;
            }
    }

    void WaveGraphicsView::handleNumberVisibileChanged(int nVisible, int scrollbarMax, int scrollbarPos)
    {
        int h = (nVisible+1) * 28;
        mMinViewportHeight = h > height() ? h : height();
        if (scrollbarMax >= 0)
        {
            verticalScrollBar()->setMaximum(scrollbarMax);
            verticalScrollBar()->setValue(scrollbarPos);
        }
//        viewport()->setMinimumHeight(mMinViewportHeight);
//SIZE        qDebug() << "numberVisibleChanged height" << mMinViewportHeight;
    }

    void WaveGraphicsView::handleViewportHeightChanged(int height)
    {
        Q_UNUSED(height);
//SIZE        qDebug() << "tree-viewportChanged height" << height << mMinViewportHeight;
        /*
        if (!height) return;
        viewport()->setFixedHeight(height);
//        qDebug() << "can scroll A" << height << verticalScrollBar()->maximum();
        verticalScrollBar()->setMaximum(height);
//        qDebug() << "can scroll B" << height << verticalScrollBar()->maximum();
*/
    }

}

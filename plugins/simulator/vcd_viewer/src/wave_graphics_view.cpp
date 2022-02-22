#include "vcd_viewer/wave_graphics_view.h"
#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_timescale.h"
#include "vcd_viewer/wave_scene.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QScrollBar>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDebug>
#include <math.h>
#include <stdio.h>

namespace hal {
    const float WaveGraphicsView::sYmag = 14;

    WaveGraphicsView::WaveGraphicsView(QWidget *parent)
        : QGraphicsView(parent), mXmag(1), mXmagMin(0.0001), mSceneLeft(0), mMinViewportHeight(1)
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
            mSceneLeft = scene()->sceneRect().left();
            adjustTimescale();
        }
    }

    void WaveGraphicsView::handelTimeframeChanged(const WaveDataTimeframe *tframe)
    {
        mSceneLeft = tframe->sceneMinTime();
        adjustTimescale();
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

        mSceneLeft = scene()->sceneRect().left();
        adjustTimescale();
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
        mSceneLeft = scene()->sceneRect().left();
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

        setTransform(QTransform(mXmag, 0, 0, 14, -mSceneLeft*mXmag, 0),false);

        Q_EMIT (changedXscale(mXmag));


        horizontalScrollBar()->setValue(xMouseScene*mXmag -ixMouseViewp);

        adjustTimescale();
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
            adjustTimescale();
    }

    void WaveGraphicsView::adjustTimescale()
    {
        int w = viewport()->width();
        mTimescale->setScale(mXmag,w,
                             mapToScene(QPoint(0,0)).x(),
                             mapToScene(QPoint(w,0)).x());

        /*
        if (horizontalScrollBar()->maximum())
        {
            horizontalScrollBar()->setMinimum(0);
            horizontalScrollBar()->setMaximum(floor ( scene()->sceneRect().width() * mXmag + 0.5));
        }
        fprintf(stderr, "Widget....%8d %8d %8d\n", geometry().x(), geometry().width(), geometry().right());
        fprintf(stderr, "Viewport..%8d %8d %8d\n", viewport()->x(), viewport()->width(), maximumViewportSize().width());
        fprintf(stderr, "Scene.....%8d %8d %8d\n", (int) scene()->sceneRect().x(), (int) scene()->sceneRect().width(), (int) scene()->sceneRect().right());
        fprintf(stderr, "Scroll....%8d %8d %8d\n", horizontalScrollBar()->minimum(), horizontalScrollBar()->value(), horizontalScrollBar()->maximum());
        fprintf(stderr, "Scaled....%8d %8d %16.8f\n", (int) floor( (horizontalScrollBar()->maximum() - horizontalScrollBar()->minimum()) / mXmag), (int) floor(horizontalScrollBar()->maximum() / mXmag), mXmag);
        fflush(stderr);
        //int maxw = viewport()->width();


        horizontalScrollBar()->setMinimum(0);
        if (horizontalScrollBar()->value() > maxw) horizontalScrollBar()->setValue(0);
        horizontalScrollBar()->setMaximum(maxw);
        */
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

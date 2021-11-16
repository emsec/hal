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
        : QGraphicsView(parent), mXmag(1), mXmagMin(0.0001)
    {
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        mTimescale = new WaveTimescale(viewport());
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

    void WaveGraphicsView::handleSizeChanged(int widgetHeight, int viewportHeight)
    {
//        qDebug() << "scene" << height() << rect().height() << "tree" << widgetHeight << viewportHeight << "scrollbar" << verticalScrollBar()->maximum();
        if (widgetHeight==viewportHeight && verticalScrollBar()->maximum() > viewportHeight)
        {
            verticalScrollBar()->setMaximum(viewportHeight);
            verticalScrollBar()->setValue(0);
        }
        if (height() == widgetHeight) return;
    }

    void WaveGraphicsView::handleViewportHeightChanged(int height)
    {
        if (!height) return;
        viewport()->setFixedHeight(height);
//        qDebug() << "can scroll A" << height << verticalScrollBar()->maximum();
        verticalScrollBar()->setMaximum(height);
//        qDebug() << "can scroll B" << height << verticalScrollBar()->maximum();
    }

}

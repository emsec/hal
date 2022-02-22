#include "waveform_viewer/wave_graphics_canvas.h"
#include "waveform_viewer/wave_scrollbar.h"
#include "waveform_viewer/wave_render_engine.h"
#include "waveform_viewer/wave_timescale.h"
#include "waveform_viewer/wave_cursor.h"
#include "waveform_viewer/wave_item.h"
#include <QScrollBar>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <math.h>

namespace hal {
    WaveGraphicsCanvas::WaveGraphicsCanvas(WaveDataList *wdlist, WaveItemHash *wHash, QWidget *parent)
        : QAbstractScrollArea(parent), mWaveDataList(wdlist), mWaveItemHash(wHash), mMoveCursor(false), mDragZoom(nullptr)
    {
        setContentsMargins(0,0,0,0);
        mTransform = WaveTransform(0, 3000, 1.);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mRenderEngine = new WaveRenderEngine(this,mWaveDataList,mWaveItemHash,viewport());
        mScrollbar = new WaveScrollbar(&mTransform,this);
        setHorizontalScrollBar(mScrollbar);
        mTimescale = new WaveTimescale(&mTransform,mScrollbar,this);
        mTimescale->show();
        mCursor = new WaveCursor(&mTransform,mScrollbar,this);
        mCursor->show();
    }

    void WaveGraphicsCanvas::resizeEvent(QResizeEvent* evt)
    {
        Q_UNUSED(evt);
        mRenderEngine->setFixedSize(viewport()->contentsRect().size());
        mScrollbar->adjust(viewport()->size().width());
        mTimescale->setScale(viewport()->size().width());
        mCursor->setViewportHeight(viewport()->size().height());
        int yScrollMax = mRenderEngine->maxHeight()-viewport()->size().height();
        verticalScrollBar()->setMaximum(yScrollMax<0 ? 0 : yScrollMax);
    }

    void WaveGraphicsCanvas::scrollContentsBy(int dx, int dy)
    {
        QAbstractScrollArea::scrollContentsBy(dx,dy);
    }

    void WaveGraphicsCanvas::mousePressEvent(QMouseEvent* evt)
    {
        if (evt->button() != Qt::LeftButton) return;

        if (mCursor->labelRect().contains(evt->pos()))
            mMoveCursor = true;
        else
        {
            mDragZoom = new QWidget(this);
            mDragZoom->setStyleSheet("background-color: rgba(255,255,255,20);");
//            mDragZoom->setAttribute(Qt::WA_StyledBackground, true);
            mDragZoom->setFixedSize(1,height()-28);
            mDragZoom->move(evt->pos().x(),28);
            mDragZoom->show();
        }
    }

    void WaveGraphicsCanvas::mouseMoveEvent(QMouseEvent* evt)
    {
        if (mMoveCursor)
        {
            mTimescale->update();
            mCursor->setCursorPosition(evt->pos());
            int xpos = evt->pos().x();
            Q_EMIT cursorMoved(mScrollbar->tPos(xpos),xpos);
        }
        else if (mDragZoom)
        {
            int x = evt->pos().x();
            QRect geo = mDragZoom->geometry();
            if (x < geo.left())
                geo.setLeft(x);
            else if (x > geo.right())
                geo.setRight(x);
            else if (x - geo.left() < geo.right() - x)
                geo.setLeft(x);
            else
                geo.setRight(x);
            mDragZoom->setFixedSize(geo.size());
            mDragZoom->move(geo.topLeft());
        }
    }

    void WaveGraphicsCanvas::mouseReleaseEvent(QMouseEvent* evt)
    {
        if (mMoveCursor)
        {
            mTimescale->update();
            mCursor->setCursorPosition(evt->pos());
            mMoveCursor = false;
            int xpos = evt->pos().x();
            Q_EMIT cursorMoved(mScrollbar->tPos(xpos),xpos);
        }
        if (mDragZoom)
        {
            QRect geo = mDragZoom->geometry();
            mDragZoom->deleteLater();
            mDragZoom = nullptr;
            double t0 = mScrollbar->tPos(geo.left());
            double dt = mScrollbar->tPos(geo.right()) - t0;
            if (dt > 0)
            {
                mTransform.setScale(viewport()->width()/dt);
                mScrollbar->adjust(viewport()->size().width());
                mScrollbar->setVleft(mTransform.vPos(t0));
                qDebug() << "zoom t0 dt vpos" << t0 << dt << mTransform.vPos(t0);
                mTimescale->setScale(viewport()->size().width());
                mRenderEngine->update();
            }
        }
    }

    void WaveGraphicsCanvas::handleTimeframeChanged(const WaveDataTimeframe* tframe)
    {
        if (mTransform.tMin() == tframe->sceneMinTime() && mTransform.tMax() == tframe->sceneMaxTime()) return;
        mTransform.setTmin(tframe->sceneMinTime());
        mTransform.setTmax(tframe->sceneMaxTime());
        mRenderEngine->update();
    }

    void WaveGraphicsCanvas::updateRequest()
    {
        mRenderEngine->update();
    }

    void WaveGraphicsCanvas::handleWaveUpdated(int iwave, int groupId)
    {
        WaveItemIndex wii(iwave, WaveItemIndex::Wire, groupId);
        WaveItem* wi = mWaveItemHash->value(wii);
        if (!wi) return;
        wi->setWaveData(mWaveDataList->at(iwave));
        mRenderEngine->update();
    }

    void WaveGraphicsCanvas::mouseDoubleClickEvent(QMouseEvent* evt)
    {
        mTimescale->update();
        mCursor->setCursorPosition(evt->pos());
        int xpos = evt->pos().x();
        Q_EMIT cursorMoved(mScrollbar->tPos(xpos),xpos);
        mMoveCursor = false;
    }

    void WaveGraphicsCanvas::wheelEvent(QWheelEvent * evt)
    {
        double oldScale = mTransform.scale();
        double newScale = oldScale;
        int dz = evt->angleDelta().y();
        if (dz > 0)
            newScale *= 1.1;
        else if (dz < 0)
            newScale /= 1.1;
        else
            return;
        if (newScale >= 100) return;
        if (newScale*mTransform.deltaT()*1.2<viewport()->width() && newScale < oldScale) return;
        int xEvent = evt->pos().x();
        double tEvent = mScrollbar->tPos(xEvent);

        mTransform.setScale(newScale);
        mScrollbar->updateScale(newScale-oldScale,tEvent,viewport()->width());
        mRenderEngine->update();
        mTimescale->setScale(viewport()->size().width());
    }

    void  WaveGraphicsCanvas::toggleZoom()
    {
        // try zoom out
        int w = viewport()->size().width();
        double dt = mTransform.deltaT();
        if (dt <= 0) return;
        double newScale = w / dt;
        double vleft = 0;
        if (mTransform.scale() <= newScale)
        {
            // already zoomed out to max, try zoom in
            TimeInterval shortest;
            for (const WaveItem* wi : mWaveItemHash->values())
            {
                if (!wi->isVisibile() || ! wi->mVisibleRange) continue;
                if (wi->mPainted.shortestToggle() < shortest) shortest = wi->mPainted.shortestToggle();
            }
            if (shortest.mCenterTime <= 0 || shortest.mDuration <= 0) return;
            newScale = 6 / shortest.mDuration;
            if (mTransform.scale() >= newScale) return;
            mTransform.setScale(newScale);
            vleft = mTransform.vPos(shortest.mCenterTime) - w/2;
            if (vleft + w > mTransform.vMax()) vleft = mTransform.vMax() - w;
            if (vleft < 0) vleft = 0;
        }
        else
            mTransform.setScale(newScale);
        mScrollbar->setVleft(vleft);
        mRenderEngine->update();
        mTimescale->setScale(viewport()->size().width());
    }

}

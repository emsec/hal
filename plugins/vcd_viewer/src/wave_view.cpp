#include "vcd_viewer/wave_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_label.h"
#include "vcd_viewer/wave_data.h"
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
    {
        WaveScene* sc = new WaveScene(this);
        connect(this,&WaveView::changedXscale,sc,&WaveScene::xScaleChanged);
        setScene(sc);

        connect(sc,&WaveScene::cursorMoved,this,&WaveView::handleCursorMoved);

        int n = sc->numberWaves();
        if (!n) return;

        for (int i=0; i<n; i++)
        {
            WaveLabel* wl = new WaveLabel(i,sc->waveData(i)->name(),this);
            connect(wl,&WaveLabel::doubleClicked,this,&WaveView::editWaveData);
            connect(wl,&WaveLabel::triggerDelete,this,&WaveView::deleteWave);
            wl->setFixedWidth(250);
            wl->show();
            mValues.append(wl);
        }
    }

    void WaveView::deleteWave(int dataIndex)
    {
        WaveScene* sc = static_cast<WaveScene*>(scene());
        WaveLabel* wl = mValues.at(dataIndex);
        mValues.removeAt(dataIndex);
        sc->deleteWave(dataIndex);
        int n = mValues.size();
        for (int i=dataIndex; i<n; i++)
        {
            float xpos = sc->cursorPos();
            mValues.at(i)->setDataIndex(i);
            updateLabel(i,xpos);
        }
        wl->deleteLater();
        mWaveIndices.clear();
        for (int i=0; i<n; i++)
            mWaveIndices.insert(sc->waveData(i)->name(),i);
    }

    void WaveView::addOrReplaceWave(WaveData* wd)
    {
        WaveScene* sc = static_cast<WaveScene*>(scene());
        auto it = mWaveIndices.find(wd->name());
        if (it != mWaveIndices.end())
        {
            sc->setWaveData(it.value(),wd);
            return;
        }
        int inx = sc->addWave(wd);
        Q_ASSERT(mValues.size() == inx);
        WaveLabel* wl = new WaveLabel(inx,sc->waveData(inx)->name(),this);
        connect(wl,&WaveLabel::doubleClicked,this,&WaveView::editWaveData);
        connect(wl,&WaveLabel::triggerDelete,this,&WaveView::deleteWave);
        wl->setFixedWidth(250);
        wl->show();
        mValues.append(wl);
        mWaveIndices.insert(wd->name(),inx);
        updateLabel(inx,sc->cursorPos());
   //     resizeEvent(nullptr);
    }

    const WaveData* WaveView::waveDataByName(const QString& name) const
    {
        int inx = mWaveIndices.value(name,-1);
        if (inx<0) return nullptr;
        WaveScene* sc = static_cast<WaveScene*>(scene());
        return sc->waveData(inx);
    }

    void WaveView::editWaveData(int dataIndex)
    {
        WaveScene* sc = static_cast<WaveScene*>(scene());
        const WaveData* editorInput = sc->waveData(dataIndex);
        if (!editorInput) return;
        WaveData wd(*editorInput);
        WaveEditDialog wed(wd,this);
        if (wed.exec() != QDialog::Accepted) return;
        sc->setWaveData(dataIndex,wed.dataFactory());
    }

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
        Q_EMIT (changedXscale(mXmag));

        WaveScene* sc = static_cast<WaveScene*>(scene());
        float xw = width()/mXmag;
        ensureVisible(QRectF(0,sceneRect().top(),xw,sceneRect().height()));
        sc->setCursorPos(xw/10,false);

        float xpos = sc->cursorPos();
        for (int i=0; i<mValues.size(); i++)
            updateLabel(i,xpos);
    }

    void WaveView::scrollContentsBy(int dx, int dy)
    {
        QGraphicsView::scrollContentsBy(dx,dy);
        if (dx) restoreCursor();
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

    void WaveView::updateLabel(int dataIndex, float xpos)
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        if (!sc) return;
        WaveLabel* wl = mValues.at(dataIndex);
        QPoint pos = mapFromScene(QPointF(xpos,sc->yPosition(dataIndex)-1));

        int ix = pos.x();

        if (ix < 0 || ix >= width())
            ix = 0;
        else
            mCursorPixelPos = ix;
        pos.setX(ix + 5);
        wl->setValue(sc->waveData(dataIndex)->tValue(xpos));
        wl->move(pos);
    }

    void WaveView::handleCursorMoved(float xpos)
    {
        WaveScene* sc = dynamic_cast<WaveScene*>(scene());
        if (!sc) return;

        if (xpos == mLastCursorPos) return;
        mLastCursorPos = xpos;

        for (int i=0; i<sc->numberWaves(); i++)
            updateLabel(i,xpos);
    }
}

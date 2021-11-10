#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/volatile_wave_data.h"
#include "vcd_viewer/wave_cursor.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QDebug>
#include <QColor>

namespace hal {
    const int WaveScene::sMinItemHeight = 4;
    const float WaveScene::sMinSceneWidth = 1000;

    WaveScene::WaveScene(WaveDataList* wdlist, VolatileWaveData *wdVol, QObject *parent)
        : QGraphicsScene(parent), mWaveDataList(wdlist), mVolatileWaveData(wdVol), mMaxPosition(0),
          mClearTimer(nullptr)
    {
        setSceneRect(0,0,sMinSceneWidth,yPosition(sMinItemHeight)+2);

        mCursor = new WaveCursor;
        addItem(mCursor);
        mCursor->setZValue(100);
        mCursor->setPos(5,5);
    }

    float WaveScene::yPosition(int irow)
    {
        return irow * 2 + 2.5;
    }

    void WaveScene::setCursorPosition(const QPointF &pos)
    {
        mCursor->setPos(pos);
    }

    float WaveScene::cursorXpostion() const
    {
        return mCursor->pos().x();
    }

    void WaveScene::handleIndexRemoved(int iwave, bool isVolatile)
    {
        QHash<int,WaveItem*>& wiHash = isVolatile ? mVolatileItems : mWaveItems;

        WaveItem* wi = nullptr;
        for (auto it = wiHash.begin(); it != wiHash.end(); it++)
        {
            if (it.value()->waveIndex() == iwave)
            {
                wi = it.value();
                wiHash.erase(it);
                break;
            }
        }

        if (wi)
        {
            removeItem(wi);
            delete wi;
        }
    }

    float WaveScene::adjustSceneRect(u64 tmax)
    {
        int n = mWaveItems.size();
        if (n<sMinItemHeight) n=sMinItemHeight;
        if (n<mMaxPosition) n=mMaxPosition;


        float maxw = sceneRect().width();
        if (maxw < sMinSceneWidth) maxw = sMinSceneWidth;
        if (maxw < tmax)           maxw = tmax;
        for (WaveItem* itm : mWaveItems)
            if (itm->boundingRect().width() > maxw)
                maxw = itm->boundingRect().width();

        // tell items that scene width changed
        if (maxw != sceneRect().width())
        {
// TODO : timescale            mTimescale->setMaxTime(maxw);
        }
        setSceneRect(QRectF(0,0,maxw,yPosition(n)+2));
        return maxw;
    }

    void WaveScene::handleWaveAdded(int iwave)
    {
        addWaveInternal(iwave,mMaxPosition++);
        adjustSceneRect();
    }

    void WaveScene::xScaleChanged(float m11)
    {
        mCursor->xScaleChanged(m11);
        mCursor->update();
    }

    void WaveScene::addWaveInternal(int iwave, int ypos)
    {
        WaveItem* wi = new WaveItem(iwave, mWaveDataList->at(iwave));
        addItem(wi);
        mWaveItems.insert(iwave,wi);
        wi->setPos(0,yPosition(ypos));
    }

    void WaveScene::setWavePositions(const QHash<int,int>& wpos)
    {
        mWavePositions = wpos;
        QHash<int,int> tempHash = wpos;

        for (auto itWave = mWaveItems.begin(); itWave != mWaveItems.end(); ++itWave)
        {
            WaveItem* wi = itWave.value();
            auto itPos = tempHash.constFind(itWave.key());
            if (itPos == tempHash.constEnd())
                wi->setVisible(false);
            else
            {
                wi->setVisible(true);
                wi->setPos(0, yPosition(itPos.value()));
                tempHash.erase(itPos);
            }
        }

        for (auto itPos = tempHash.begin(); itPos != tempHash.end(); ++itPos)
        {
            addWaveInternal(itPos.key(), itPos.value());
        }
    }

    void WaveScene::emitCursorMoved(float xpos)
    {
        if (xpos < sceneRect().left() || xpos > sceneRect().right()) return;
        Q_EMIT (cursorMoved(xpos));
        if (mClearTimer)
        {
            mClearTimer->stop();
            mClearTimer->deleteLater();
        }
        mClearTimer = new QTimer(this);
        mClearTimer->setSingleShot(true);
        connect(mClearTimer,&QTimer::timeout,this,&QGraphicsScene::clearSelection);
        mClearTimer->start(50);
    }


    void WaveScene::handleVolatileRepaint()
    {
        for (WaveItem* vit : mVolatileItems.values())
        {
            removeItem(vit);
            delete vit;
        }
        mVolatileItems.clear();

        for (const VolatileWaveData::VolatileWaveItem& vwi : mVolatileWaveData->yPositionData())
        {
            WaveItem* vit = new WaveItem(vwi.groupId,vwi.wd);
            addItem(vit);
            vit->setPos(0, yPosition(vwi.yPosition));
            mVolatileItems.insert(vwi.yPosition,vit);
        }
    }
}

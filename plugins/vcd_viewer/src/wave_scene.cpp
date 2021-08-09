#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/wave_data.h"
#include "vcd_viewer/wave_cursor.h"
#include "vcd_viewer/wave_timescale.h"
#include "vcd_viewer/vcd_serializer.h"
#include <QDebug>

namespace hal {

    WaveScene::WaveScene(QObject* parent)
        : QGraphicsScene(parent), mClearTimer(nullptr)
    {
        setSceneRect(QRectF(0,0,1000,5));
        mCursor = new WaveCursor();
        addItem(mCursor);

        mTimescale = new WaveTimescale(6000);
        addItem(mTimescale);
    }

    WaveScene::~WaveScene()
    {
        // TODO
    }

    void WaveScene::generateDataSample()
    {
        int freq[5] = {50, 100, 200, 400, 800};

        QList<const WaveData*> wlist;
        for (int i=0; i<5; i++)
            wlist.append(WaveData::clockFactory(0, freq[i], 2000));

        VcdSerializer writer(this);
        writer.serialize("dummy.vcd", wlist);
        QList<WaveData*> wparse = writer.deserialize("dummy.vcd");

        for (WaveData* wd : wparse)
        {
            addWave(wd);
        }
    }

    int WaveScene::addWave(WaveData* wd)
    {
        int inx = mWaveItems.size();
        WaveItem* wi = new WaveItem(wd, yPosition(inx));
        addItem(wi);
        mWaveItems.append(wi);

        float maxw = adjustSceneRect();

        wi->setSceneMax(maxw);

        return inx;
    }

    float WaveScene::adjustSceneRect()
    {
        int n = mWaveItems.size();
        float maxw = 1000;
        for (WaveItem* itm : mWaveItems)
            if (itm->boundingRect().width() > maxw)
                maxw = itm->boundingRect().width();
        setSceneRect(QRectF(0,0,maxw,yPosition(n)+4));
        return maxw;
    }

    void WaveScene::moveToIndex(int indexFrom, int indexTo)
    {
        if (indexFrom==indexTo) return;
        WaveItem* wi = mWaveItems.at(indexFrom);
        mWaveItems.removeAt(indexFrom);
        mWaveItems.insert(indexTo < indexFrom ? indexTo : indexTo-1, wi);
        int i0 = indexFrom < indexTo ? indexFrom : indexTo;
        int i1 = indexFrom < indexTo ? indexTo : indexFrom;
        for (int i= i0; i<=i1; i++)
        {
            WaveItem* wii = mWaveItems.at(i);
            wii->setYoffset(yPosition(i));
            wii->update();
        }
    }

    void WaveScene::xScaleChanged(float m11)
    {
        mTimescale->xScaleChanged(m11);
        mTimescale->update();
        mCursor->xScaleChanged(m11);
        mCursor->update();
   }

    float WaveScene::cursorPos() const
    {
        return mCursor->pos().x();
    }

    void WaveScene::setCursorPos(float xp, bool relative)
    {
        if (relative)
            mCursor->setPos(mCursor->pos() + QPointF(xp,0));
        else
            mCursor->setPos(xp,0);
    }

    float WaveScene::yPosition(int dataIndex) const
    {
        return (dataIndex+2)*4;
    }

    void WaveScene::deleteWave(int dataIndex)
    {
        WaveItem* wiDelete = mWaveItems.at(dataIndex);
        mWaveItems.removeAt(dataIndex);
        removeItem(wiDelete);
        wiDelete->update();
        int n = mWaveItems.size();
        for (int i= dataIndex; i<n; i++)
        {
            WaveItem* wi = mWaveItems.at(i);
            wi->setYoffset(yPosition(i));
            wi->update();
        }
        update();
        delete wiDelete;
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

    const WaveData* WaveScene::waveData(int dataIndex) const
    {
        if (dataIndex >= mWaveItems.size()) return nullptr;
        return mWaveItems.at(dataIndex)->wavedata();
    }

    void WaveScene::setWaveData(int dataIndex, WaveData* wd)
    {
        Q_ASSERT(dataIndex < mWaveItems.size());
        WaveItem* wi = mWaveItems.at(dataIndex);
        wi->setWavedata(wd);
        float wmax = adjustSceneRect();
        wi->setSceneMax(wmax);
        wi->update();
        update();
        Q_EMIT (cursorMoved(cursorPos()));
    }

}

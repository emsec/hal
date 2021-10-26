#include "vcd_viewer/wave_scene.h"
#include <QGraphicsRectItem>
#include "vcd_viewer/wave_item.h"
#include "netlist_simulator_controller/wave_data.h"
#include "vcd_viewer/wave_cursor.h"
#include "vcd_viewer/wave_timescale.h"
#include "vcd_viewer/wave_index.h"
#include "netlist_simulator_controller/vcd_serializer.h"
#include <QDebug>

namespace hal {

    const int WaveScene::sMinItemHeight = 5;
    const float WaveScene::sMinSceneWidth = 1000;

    WaveScene::WaveScene(const WaveIndex *winx, QObject* parent)
        : QGraphicsScene(parent), mWaveIndex(winx), mClearTimer(nullptr), mDebugSceneRect(nullptr)
    {
        setSceneRect(QRectF(0,0,sMinSceneWidth,yPosition(sMinItemHeight)+4));

        /*
        mDebugSceneRect = new QGraphicsRectItem(sceneRect());
        mDebugSceneRect->setPen(QPen(Qt::red,0));
        addItem(mDebugSceneRect);
        */
        mCursor = new WaveCursor();
        addItem(mCursor);

        mTimescale = new WaveTimescale(sMinSceneWidth);
        addItem(mTimescale);
        connect(mWaveIndex->waveDataList(),&WaveDataList::maxTimeChanged,this,&WaveScene::handleMaxTimeChanged);
    }

    WaveScene::~WaveScene()
    {
        // TODO
    }

    void WaveScene::generateDataSample()
    {
        /*
        int freq[5] = {50, 100, 200, 400, 800};
        u32 ids[5] = {9, 7, 6, 4, 8};

        QList<const WaveData*> wlist;
        for (int i=0; i<5; i++)
            wlist.append(WaveData::clockFactory(ids[i], QString("INPUT_%1[%2]").arg(i).arg(ids[i]), 0, freq[i], 2000));

        VcdSerializer writer(this);
        writer.serialize("fsmin.vcd", wlist);
        QList<WaveData*> wparse = writer.deserialize("fsmin.vcd");

        for (WaveData* wd : wparse)
        {
            addWave(wd);
        }
        */
    }

    void WaveScene::handleWaveAppended(WaveData *wd)
    {
        qDebug() << "handleWaveAppended" << wd->id() << wd->name() << wd->size() << hex << (quintptr) wd;
        addWave(wd);
    }

    void WaveScene::handleWaveDataChanged(int inx)
    {
        int i0 = inx<0 ? 0 : inx;
        int i1 = inx<0 ? mWaveItems.size() : inx+1;
        for (int i=i0; i<i1; i++)
        {
            WaveItem* wi = mWaveItems.at(i);
            WaveData* wd = mWaveIndex->waveData(i);
            qDebug() << "handleWaveChanged" << i << wd->id() << wd->name() << wd->size() << hex << (quintptr) wd;
            wi->setWavedata(wd);
            wi->update();
        }
        update();
    }

    void WaveScene::handleWaveRemoved(int inx)
    {
        if (inx < 0)
        {
            for (WaveItem* wi : mWaveItems)
            {
                removeItem(wi);
                delete wi;
            }
            mWaveItems.clear();
            update();
        }
        else
            deleteWave(inx);
    }

    int WaveScene::addWave(WaveData* wd)
    {
        int inx = mWaveItems.size();
        WaveItem* wi = new WaveItem(wd, yPosition(inx));
        addItem(wi);
        mWaveItems.append(wi);
        adjustSceneRect();

        // TODO : y-scroll to make new wave visible
        return inx;
    }

    float WaveScene::adjustSceneRect(u64 tmax)
    {
        // TODO : min lines = 5 as const
        int n = mWaveItems.size();
        if (n<5) n=5;


        float maxw = sceneRect().width();
        if (maxw < sMinSceneWidth) maxw = sMinSceneWidth;
        if (maxw < tmax)           maxw = tmax;
        for (WaveItem* itm : mWaveItems)
            if (itm->boundingRect().width() > maxw)
                maxw = itm->boundingRect().width();

        // tell items that scene width changed
        if (maxw != sceneRect().width())
        {
            for (WaveItem* itm : mWaveItems)
                itm->setMaxTime(maxw);
            mTimescale->setMaxTime(maxw);
        }
        setSceneRect(QRectF(0,0,maxw,yPosition(n)+4));
        if (mDebugSceneRect)
            mDebugSceneRect->setRect(sceneRect());
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
        int inx = dataIndex < 0 ? mWaveItems.size() : dataIndex;
        return (inx+2)*4;
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

    void WaveScene::handleMaxTimeChanged(u64 tmax)
    {
        adjustSceneRect(tmax);
    }

    void WaveScene::setWaveData(int dataIndex, WaveData* wd)
    {
        Q_ASSERT(dataIndex < mWaveItems.size());
        WaveItem* wi = mWaveItems.at(dataIndex);
        wi->setWavedata(wd);
        wi->update();
        update();
        Q_EMIT (cursorMoved(cursorPos()));
    }

}

#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/wave_cursor.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QDebug>
#include <QColor>

namespace hal {
    const int WaveScene::sMinItemHeight = 4;
    const float WaveScene::sMinSceneWidth = 1000;

    WaveScene::WaveScene(WaveDataList* wdlist, QObject *parent)
        : QGraphicsScene(parent), mWaveDataList(wdlist), mMaxPosition(0),
          mClearTimer(nullptr), mXmag(1)
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

    float WaveScene::cursorXposition() const
    {
        return mCursor->pos().x();
    }

    void WaveScene::handleIndexRemoved(int iwave, bool isGroup)
    {
        QHash<int,WaveItem*>& wiHash = isGroup ? mGroupItems : mWaveItems;

        auto it = wiHash.find(iwave);
        if (it == wiHash.end()) return;
        WaveItem* wi = it.value();
        wiHash.erase(it);

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
        float h = yPosition(n)+2;

        if (maxw != sceneRect().width() || h != sceneRect().height())
        {
            setSceneRect(QRectF(0,0,maxw,h));
        }
        return maxw;
    }

    void WaveScene::handleWaveAdded(int iwave)
    {
        addWaveInternal(iwave,mMaxPosition++);
        adjustSceneRect();
    }

    void WaveScene::handleGroupAdded(int grpId)
    {
        addGroupInternal(grpId,mMaxPosition++);
        adjustSceneRect();
    }

    void WaveScene::handleWaveUpdated(int iwave)
    {
        if (mWaveItems.find(iwave) == mWaveItems.end()) return;
        adjustSceneRect();
    }

    void WaveScene::xScaleChanged(float m11)
    {
        if (m11 == mXmag) return;
        mXmag = m11;
        mCursor->xScaleChanged(mXmag);
        mCursor->update();
        for (WaveItem* itm : mGroupItems)
            itm->updateScaleFactor(mXmag);
    }

    void WaveScene::addWaveInternal(int iwave, int ypos)
    {
        WaveItem* wi = new WaveItem(iwave, mWaveDataList->at(iwave));
        addItem(wi);
        mWaveItems.insert(iwave,wi);
        wi->setPos(0,yPosition(ypos));
    }

    void WaveScene::addGroupInternal(int grpId, int ypos)
    {
        WaveDataGroup* grp = mWaveDataList->mDataGroups.value(grpId);
        if (!grp) return;
        WaveItem* wi = new WaveItem(grpId, grp);
        addItem(wi);
        mGroupItems.insert(grpId,wi);
        wi->setPos(0,yPosition(ypos));
    }

    void WaveScene::setWavePositions(const QHash<int,int>& wpos, const QHash<int,int>& gpos)
    {
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

        tempHash = gpos;
        for (auto itGrp = mGroupItems.begin(); itGrp != mGroupItems.end(); ++itGrp)
        {
            WaveItem* wi = itGrp.value();
            auto itPos = tempHash.constFind(itGrp.key());
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
            addGroupInternal(itPos.key(), itPos.value());
        }
        adjustSceneRect();
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

}

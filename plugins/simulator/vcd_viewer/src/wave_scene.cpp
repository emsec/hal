#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/wave_cursor.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QDebug>
#include <QColor>

namespace hal {
    const int WaveScene::sMinItemHeight = 4;
    const float WaveScene::sMinSceneWidth = 1000;

    WaveScene::WaveScene(WaveDataList* wdlist, WaveItemHash *wHash, QObject *parent)
        : QGraphicsScene(parent), mWaveDataList(wdlist), mWaveItemHash(wHash),
          mClearTimer(nullptr), mXmag(1)
    {
        setSceneRect(0,0,sMinSceneWidth,yPosition(sMinItemHeight)+2);

        /* debug scene rect
        mSceneRect = new QGraphicsRectItem(sceneRect());
        mSceneRect->setBrush(QColor::fromRgb(20,40,80));
        addItem(mSceneRect);
        */

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

    void WaveScene::updateWaveItems()
    {
        // height
        int n = nextWavePosition();
        if (n<sMinItemHeight) n=sMinItemHeight;
        float h = yPosition(n)+2;
        QRectF scr = sceneRect();
        if (scr.height() < h)
        {
            scr.setHeight(h);
            setSceneRect(scr);
  //          mSceneRect->setRect(sceneRect());
        }

//        mWaveItemHash->dump("scene");
        auto it = mWaveItemHash->begin();
        while (it != mWaveItemHash->end())
        {
            WaveItem* wi = it.value();

            if (wi->hasRequest(WaveItem::AddRequest))
            {
                addItem(wi);
                wi->clearRequest(WaveItem::AddRequest);
            }

            if (wi->hasRequest(WaveItem::DeleteRequest))
            {
                removeItem(wi);
                wi->clearRequest(WaveItem::DeleteRequest);
                wi->setRequest(WaveItem::DeleteAcknowledged);
            }

            if (wi->hasRequest(WaveItem::DeleteAcknowledged))
            {
                delete wi;
                it = mWaveItemHash->erase(it);
            }
            else
            {
                if (wi->hasRequest(WaveItem::SetPosition))
                {
                    wi->setPos(0, yPosition(wi->yPosition()));
                    wi->clearRequest(WaveItem::SetPosition);
                }


                if (wi->hasRequest(WaveItem::DataChanged))
                {
                    wi->update();
                }

                if (wi->hasRequest(WaveItem::SetVisible) && wi->scene())
                {
                    wi->setVisible(wi->waveVisibile());
                    wi->clearRequest(WaveItem::SetVisible);
                }
                ++it;
            }
        }
    }

    int WaveScene::nextWavePosition() const
    {
        return mWaveItemHash->size();
    }

    void WaveScene::handleMaxTimeChanged(u64 tmax)
    {
        if (tmax > sceneRect().width())
            adjustSceneRect(tmax);
    }


    float WaveScene::adjustSceneRect(u64 tmax)
    {
        // height
        int n = nextWavePosition();
        if (n<sMinItemHeight) n=sMinItemHeight;
        float h = yPosition(n)+2;

        // width
        float maxw = sceneRect().width();
        if (maxw < sMinSceneWidth) maxw = sMinSceneWidth;
        if (maxw < tmax)           maxw = tmax;
        for (WaveItem* itm : mWaveItemHash->values())
            if (itm->boundingRect().width() > maxw)
                maxw = itm->boundingRect().width();

        // tell items that scene width changed

        if (maxw != sceneRect().width() || h != sceneRect().height())
        {
            setSceneRect(QRectF(0,0,maxw,h));
//            mSceneRect->setRect(sceneRect());
        }
        return maxw;
    }

    void WaveScene::handleWaveUpdated(int iwave, int groupId)
    {
        WaveItemIndex wii(iwave, WaveItemIndex::Wire, groupId);
        WaveItem* wi = mWaveItemHash->value(wii);
        if (!wi) return;
        wi->setWaveData(mWaveDataList->at(iwave));
        wi->update();
        adjustSceneRect();
    }

    void WaveScene::updateWaveItemValues()
    {
        // TODO : base value changed
    }

    void WaveScene::xScaleChanged(float m11)
    {
        if (m11 == mXmag) return;
        mXmag = m11;
        mCursor->xScaleChanged(mXmag);
        mCursor->update();
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

#include <QPainter>
#include <QDebug>
#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/wave_scene.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QGraphicsScene>
#include <math.h>
#include <QColor>

namespace hal {

const char* WaveItem::sLineColor[] = { "#10E0FF", "#C08010"} ;

    WaveItem::WaveItem(int iwave, WaveData *dat)
        : mWaveIndex(iwave), mData(dat), mMaxTime(WaveScene::sMinSceneWidth), mInactive(false)
    {
        construct();
        if (mData) mData->setGraphicsItem(this);
    }

    WaveItem::~WaveItem()
    {
        mData->setGraphicsItem(nullptr);
    }

    void WaveItem::construct()
    {
        prepareGeometryChange();
        mSolidLines.clear();
        mDotLines.clear();

        if (!childItems().isEmpty())
        {
            for (QGraphicsItem* cit : childItems())
                delete cit;
            childItems().clear();
        }

        if (!mData)
        {
            return;
        }

        auto lastIt = mData->data().constEnd();
        for (auto nextIt = mData->data().constBegin(); nextIt != mData->data().constEnd(); ++nextIt)
        {
            u64 t1 = nextIt.key();
            if (t1 > mMaxTime) mMaxTime = t1;
            if (lastIt != mData->data().constEnd())
            {
                u64 t0 = lastIt.key();
                if (lastIt.value() < 0)
                {
                    float ydot = 0.5;
                    mDotLines.append(QLineF(t0,ydot,t1,ydot));
                }
                else
                {
                    int lastVal = 1. - lastIt.value();
                    if (mData->bits()==1 || !lastIt.value())
                    {
                        mSolidLines.append(QLine(t0,lastVal,t1,lastVal)); // hline

                        if (nextIt.value() >= 0)
                        {
                            int nextVal = 1. - nextIt.value();
                            if (mData->bits()==1 || !nextIt.value())
                                mSolidLines.append(QLine(t1,lastVal,t1,nextVal)); // vline
                        }
                    }
                }
            }
            lastIt = nextIt;
        }

        if (mData->bits() > 1)
        {
            float m11 = 1;
            const WaveScene* sc = static_cast<WaveScene*>(scene());
            if (sc) m11 = sc->xScaleFactor();

            for (auto it = mData->data().constBegin(); it != mData->data().constEnd();)
            {
                u64 t0 = it.key();
                int v0 = it.value();
                ++it;
                if (v0 > 0)
                {
                    u64 t1 = it == mData->data().constEnd() ? mMaxTime : it.key();
                    float w = (t1 < t0+10 ? 10 : t1-t0);
                    WaveValueAsTextItem* wvti = new WaveValueAsTextItem(v0,w,m11,this);
                    wvti->setPos(t0,0);
                }
            }

        }
    }

    void WaveItem::updateGraphicsItem(WaveData *wd)
    {
        mData = wd;
        if (mData) mData->setGraphicsItem(this);
        construct();
    }

    void WaveItem::removeGraphicsItem()
    {
        if (scene()) scene()->removeItem(this);
    }

    void WaveItem::setItemVisible(bool vis)
    {
        setVisible(vis);
    }

    void WaveItem::updateScaleFactor(float m11)
    {
        if (childItems().isEmpty()) return;
        for (QGraphicsItem* cit : childItems())
        {
            WaveValueAsTextItem* wvti = static_cast<WaveValueAsTextItem*>(cit);
            wvti->updateScaleFactor(m11);
        }
    }

    void WaveItem::aboutToBeDeleted()
    {
//        mMaxTime = 1;
//        prepareGeometryChange();
        mInactive = true;
    }

    float WaveItem::maxTime() const
    {
        float retval = WaveScene::sMinSceneWidth;
        if (wavedata()->maxTime() > retval) retval = wavedata()->maxTime();
        if (scene() && scene()->sceneRect().width() > retval) retval = scene()->sceneRect().width();
        return retval;
    }

    void WaveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        if (mInactive) return;

        float x1 = maxTime();
        if (x1 != mMaxTime)
        {
            mMaxTime = x1;
            if (mData->netType() == WaveData::ClockNet)
                construct();
            else
                prepareGeometryChange();
        }

        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(QPen(QBrush(QColor(sLineColor[1])),0.,Qt::DotLine)); // TODO : style
        painter->drawLines(mDotLines);

        painter->setPen(QPen(QBrush(QColor(sLineColor[0])),0.));  // TODO : style
        painter->drawLines(mSolidLines);

        float  y = 1;

        if (mData->data().isEmpty())
        {
            painter->setPen(QPen(QBrush(QColor(sLineColor[1])),0.,Qt::DotLine));
            y -= 0.5;
            painter->drawLine(QLineF(0,y,x1,y));
            return;
        }

        if (mData->bits() > 1)
        {
            for (auto it = mData->data().constBegin(); it!=mData->data().constEnd();)
            {
                u64 t0 = it.key();
                int v0 = it.value();
                ++it;
                if (v0>0)
                {
                    u64 t1 = it==mData->data().constEnd() ? mMaxTime : it.key();
                    QRectF r(t0,-0.05,t1-t0,1.1);
                    painter->drawRoundedRect(r,50.0,0.5);
                }
            }
        }

        float x0 = mData->maxTime();
        if (x0 < x1)
        {
            if (mData->data().last() < 0)
            {
                y -= 0.5;
                painter->setPen(QPen(QBrush(QColor(sLineColor[1])),0.,Qt::DotLine));
            }
            else
                y -= mData->data().last();
            painter->drawLine(QLineF(x0,y,x1,y));
        }
    }

    QRectF WaveItem::boundingRect() const
    {
        return QRectF(0,-0.05,mMaxTime,1.1);
    }

    WaveValueAsTextItem::WaveValueAsTextItem(int val, float w, float m11, QGraphicsItem *parentItem)
        : QGraphicsItem(parentItem), mValue(val), mWidth(w), mXmag(m11)
    {
//        setFlags(flags() | QGraphicsItem::ItemIgnoresTransformations);
    }

    void WaveValueAsTextItem::updateScaleFactor(float m11)
    {
        if (mXmag==m11) return;
        mXmag = m11;
        update();
    }

    void WaveValueAsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        const WaveItem* pItem = static_cast<const WaveItem*>(parentItem());
        QString txt = pItem->wavedata()->strValue(mValue);
        painter->setTransform(QTransform(1/mXmag,0,0,1/14.,0,0),true);
        QRectF rTrans(0,0,boundingRect().width()*mXmag,boundingRect().height()*14);
        QFont font = painter->font();
        font.setPixelSize(11);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QPen(Qt::white,0)); // TODO : style
        float textWidth = painter->boundingRect(rTrans,Qt::AlignHCenter | Qt::AlignVCenter, txt).width();
        if (textWidth + 6 <= rTrans.width())
            painter->drawText(rTrans,Qt::AlignHCenter | Qt::AlignVCenter, txt);
    }

    QRectF WaveValueAsTextItem::boundingRect() const
    {
        return QRectF(0,0,mWidth,1);
    }
}

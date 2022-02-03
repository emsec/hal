#include <QPainter>
#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/wave_scene.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QGraphicsScene>
#include <QTextStream>
#include <math.h>
#include <QColor>
#include <QScrollBar>

namespace hal {

    const char* WaveItem::sLineColor[] = { "#10E0FF", "#60F0FF", "#C08010", "#0D293E"} ;

    bool WaveItem::sValuesAsText = false;

    WaveItem::WaveItem(WaveData *dat)
        : mData(dat), mYposition(-1), mRequest(0), mMinTime(0),
          mMaxTime(WaveScene::sMinSceneWidth), mMaxTransition(0),
          mVisibile(true), mSelected(false)
    {
        construct();
    }

    WaveItem::~WaveItem()
    {
    }

    void WaveItem::setYposition(int pos)
    {
        if (mYposition == pos) return;
        mYposition = pos;
        setRequest(SetPosition);
    }

    void WaveItem::setWaveData(WaveData* wd)
    {
        mData = wd;
        setRequest(DataChanged);
    }

    void WaveItem::setWaveVisible(bool vis)
    {
        if (mVisibile==vis) return;
        mVisibile = vis;
        setRequest(SetVisible);
    }

    void WaveItem::setWaveSelected(bool sel)
    {
        if (mSelected==sel) return;
        mSelected = sel;
        setRequest(SelectionChanged);
    }

    void WaveItem::enforceYposition()
    {
        setPos(0,WaveScene::yPosition(mYposition));
        clearRequest(SetPosition);
    }

    void WaveItem::construct()
    {
        prepareGeometryChange();
        mSolidLines.clear();
        mDotLines.clear();
        mGrpRects.clear();
        mMaxTransition = 0;

        clearRequest(DataChanged);
        clearRequest(SelectionChanged);

        if (!childItems().isEmpty())
        {
            for (QGraphicsItem* cit : childItems())
                delete cit;
            childItems().clear();
        }

        if (!mData) return;
        mData->setDirty(false);

        if (mData->data().isEmpty()) return;

        if (mData->bits()>1)
            constructGroup();
        else
            constructWire();
    }

    void WaveItem::constructWire()
    {
        auto lastIt = mData->data().constEnd();
        for (auto nextIt = mData->data().constBegin(); nextIt != mData->data().constEnd(); ++nextIt)
        {
            u64 t1 = nextIt.key();
            if (t1 > mMaxTime) t1 = mMaxTime;
            if (t1 > mMaxTransition) mMaxTransition = t1;
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
                    mSolidLines.append(QLine(t0,lastVal,t1,lastVal)); // hline

                    if (nextIt.value() >= 0)
                    {
                        int nextVal = 1. - nextIt.value();
                        mSolidLines.append(QLine(t1,lastVal,t1,nextVal)); // vline
                    }
                }
            }
            lastIt = nextIt;
        }
    }

    void WaveItem::constructGroup()
    {
        for (auto it = mData->data().constBegin(); it != mData->data().constEnd();)
        {
            u64 t0 = it.key();
            int v0 = it.value();
            ++it;
            u64 t1 = it == mData->data().constEnd() ? mMaxTime : it.key();
            if (t1 > mMaxTransition) mMaxTransition = t1;
            float w = (t1 < t0+10 ? 10 : t1-t0);
            if (sValuesAsText)
            {
                WaveValueAsTextItem* wvti = new WaveValueAsTextItem(v0,w,this);
                wvti->setPos(t0,0);
            }
            else
                mGrpRects.append(QRectF(t0,0,w,1));
        }
    }

    bool WaveItem::setTimeframe()
    {
        float tmin, tmax;
        if (scene() && scene()->sceneRect().width() > 1)
        {
            tmin = scene()->sceneRect().left();
            tmax = scene()->sceneRect().right();
        }
        else
        {
            tmin = 0;
            tmax = 1000;
        }
        if (tmin == mMinTime && tmax == mMaxTime) return false;
        mMinTime = tmin;
        mMaxTime = tmax;
        return true;
    }

    void WaveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        if (hasRequest(DeleteAcknowledged))
            return;

        if (hasRequest(SetPosition))
            enforceYposition();

        if (mSelected)
        {
            painter->setBrush(QBrush(sLineColor[Background]));
            painter->setPen(Qt::NoPen);
            painter->drawRect(boundingRect());
            painter->setBrush(Qt::NoBrush);
        }

        bool bboxChanged = setTimeframe();

        if (hasRequest(DataChanged) || hasRequest(SelectionChanged) || bboxChanged)
        {
            construct();
            clearRequest(DataChanged);
            clearRequest(SelectionChanged);
        }
        else
        {
            /*
            if (false)  // TODO
            {
                if (mData->netType() == WaveData::ClockNet)
                    construct();
                else
                    prepareGeometryChange();
            }
            else if (mData && mData->bits() > 1)
            {
            */
                if (sValuesAsText)
                {
                    if (!mGrpRects.isEmpty()) construct();
                }
                else
                {
                    if (!childItems().isEmpty()) construct();
                }
            // }
        }

        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(QPen(QBrush(QColor(sLineColor[Dotted])),0.,Qt::DotLine)); // TODO : style
        painter->drawLines(mDotLines);

        painter->setPen(QPen(QBrush(QColor(sLineColor[mSelected?HiLight:Solid])),0.));  // TODO : style
        painter->drawLines(mSolidLines);

        float  y = 1;

        if (mData->data().isEmpty())
        {
            painter->setPen(QPen(QBrush(QColor(sLineColor[Dotted])),0.,Qt::DotLine));
            y -= 0.5;
            painter->drawLine(QLineF(mMinTime,y,mMaxTime,y));
            return;
        }

        if (sValuesAsText &&  mData->bits() > 1)
        {
            for (auto it = mData->data().constBegin(); it!=mData->data().constEnd();)
            {
                u64 t0 = it.key();
       //         int v0 = it.value();
                ++it;
                u64 t1 = it==mData->data().constEnd() ? mMaxTime : it.key();
                QRectF r(t0,-0.05,t1-t0,1.1);
                painter->drawRoundedRect(r,50.0,0.5);
            }
        }

        if (mMaxTransition < mMaxTime)
        {
            if (mData->data().last() < 0)
            {
                y -= 0.5;
                painter->setPen(QPen(QBrush(QColor(sLineColor[Dotted])),0.,Qt::DotLine));
            }
            else
                y -= mData->data().last();
            painter->drawLine(QLineF(mMaxTransition,y,mMaxTime,y));
        }

        if (!sValuesAsText && !mGrpRects.isEmpty())
        {
            painter->setPen(QPen(QBrush(QColor(sLineColor[mSelected?HiLight:Solid])),0.));
            painter->setBrush(Qt::NoBrush);
            painter->drawRects(mGrpRects);
        }
    }

    QRectF WaveItem::boundingRect() const
    {
        return QRectF(mMinTime,-0.2,mMaxTime-mMinTime,1.4);
    }

    bool WaveItem::hasRequest(Request rq) const
    {
        int mask = 1 << rq;
        return (mRequest & mask) != 0;
    }

    void WaveItem::setRequest(Request rq)
    {
        int mask = 1 << rq;
        mRequest |= mask;
    }

    void WaveItem::clearRequest(Request rq)
    {
        int mask = ~(1 << rq);
        mRequest &= mask;
    }

    bool WaveItem::isDeleted() const
    {
        return hasRequest(DeleteRequest) || hasRequest(DeleteAcknowledged);
    }

    WaveValueAsTextItem::WaveValueAsTextItem(int val, float w, QGraphicsItem *parentItem)
        : QGraphicsItem(parentItem), mValue(val), mWidth(w)
    {
//        setFlags(flags() | QGraphicsItem::ItemIgnoresTransformations);
    }

    void WaveValueAsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        float m11 = 1;
        const WaveScene*sc = static_cast<const WaveScene*>(scene());
        if (sc) m11 = sc->xScaleFactor();
        float w = m11 * boundingRect().width();
        float dx = sc->sceneRect().left();
        if (w<10) return;
        const WaveItem* pItem = static_cast<const WaveItem*>(parentItem());
        QString txt = pItem->wavedata()->strValue(mValue);
        painter->setTransform(QTransform(1/m11,0,0,1/14.,dx,0),true);
        QRectF rTrans(0,0,w,boundingRect().height()*14);
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
        return QRectF(0,-0.05,mWidth,1.1);
    }

    bool WaveItemIndex::operator==(const WaveItemIndex &other) const
    {
        return mType == other.mType && mIndex == other.mIndex && mParentId == other.mParentId;
    }

    uint qHash(const WaveItemIndex& wii)
    {
        if (!wii.isValid()) return 0;
        return (wii.parentId() << 20) | ((wii.index()+1) << 1) | (wii.isGroup() ? 1 : 0);
    }

    int WaveItemHash::importedWires() const
    {
        QSet<u32> ids;
        for (const WaveItemIndex& wii : keys())
        {
            if (wii.isWire()) ids.insert(wii.index());
        }
        return ids.size();
    }

    void WaveItemHash::addOrReplace(WaveData*wd, WaveItemIndex::IndexType tp, int inx, int parentId)
    {
        WaveItemIndex wii(inx, tp, parentId);
        WaveItem* wi = value(wii);
        int ypos = -1;
        if (wi)
        {
            ypos = wi->yPosition();
            if (wi->scene()) wi->scene()->removeItem(wi);
            delete wi;
        }

        wi = new WaveItem(wd);
        if (ypos >= 0) wi->setYposition(ypos);
        wi->setRequest(WaveItem::AddRequest);
        operator[] (wii) = wi;
    }

    void WaveItemHash::dump(const char* stub)
    {
        const char* req = "+CVPS-D";
        QGraphicsScene* sc = nullptr;
        for (WaveItem* wi : values())
        {
            if (wi->scene())
            {
                sc = wi->scene();
                break;
            }
        }

        QSet<QGraphicsItem*> sceneItems;
        QTextStream xout(stderr, QIODevice::WriteOnly);
        if (sc)
        {
            for (QGraphicsItem* gi : sc->items()) sceneItems.insert(gi);

            xout << "---" << stub << "------["
                 << sc->sceneRect().x() << ","
                 << sc->sceneRect().y() << ","
                 << sc->sceneRect().width() << ","
                 << sc->sceneRect().height() << "]---\n";
        }
        else
            xout << "---" << stub << "------\n";

       for (auto it=constBegin(); it!=constEnd(); ++it)
        {
            xout << (sceneItems.contains(it.value()) ? '*' : ' ');
            xout << hex << (quintptr) it.value() << " " << dec;
            for (int i=0; i<6; i++)
                xout << (it.value()->hasRequest((WaveItem::Request)i) ? req[i] : '.');
            xout << (it.value()->waveVisibile() ? " +++ " : " --- " );
            xout << (it.key().isGroup() ? "G " : "W ");
            xout << (it.value()->yPosition() < 0 ? QString(".") : QString::number(it.value()->yPosition()))
                 << "[" << it.key().index() << "," << it.key().parentId() << "] -> "
                 << (it.value()->isDeleted() ? QString("_(deleted)_") : it.value()->wavedata()->name());
            xout << "\n";
        }
       xout << "hash=" << size() << "   scene=" << sceneItems.size() << "\n";
    }
}

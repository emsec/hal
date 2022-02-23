#include <QPainter>
#include "waveform_viewer/wave_item.h"
#include "waveform_viewer/wave_data_provider.h"
#include "waveform_viewer/wave_render_engine.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QGraphicsScene>
#include <QTextStream>
#include <math.h>
#include <QColor>
#include <QScrollBar>
#include <QDebug>
#include <QThread>

namespace hal {

    const char* WaveItem::sLineColor[] = { "#10E0FF", "#60F0FF", "#C08010", "#0D293E"} ;

    bool WaveItem::sValuesAsText = false;

    WaveItem::WaveItem(WaveData *dat, QObject *parent)
        : QObject(parent), mData(dat), mLoader(nullptr), mLoadProgress(0), mState(Null),
          mVisibleRange(false), mLoop(false),
          mYposition(-1), mRequest(0), mMinTime(0),
          mMaxTime(1000), mMaxTransition(0),
          mVisibile(true), mSelected(false)
    {;}

    WaveItem::~WaveItem()
    {
        qDebug() << "delete wave item" << mData->name();
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

    bool WaveItem::setTimeframe()
    {
        float tmin, tmax;
        /* TODO
        if (scene() && scene()->sceneRect().width() > 1)
        {
            tmin = scene()->sceneRect().left();
            tmax = scene()->sceneRect().right();
        }
        else
        {
        */
            tmin = 0;
            tmax = 1000;
        //}
        if (tmin == mMinTime && tmax == mMaxTime) return false;
        mMinTime = tmin;
        mMaxTime = tmax;
        return true;
    }


    void WaveItem::deletePainted()
    {
        mPainted.clearPrimitives();
        setState(Null);
    }

    void WaveItem::startGeneratePainted(const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar)
    {
        // precondition: mutex lock is set, state is Null
        mLoadProgress = 0;

        if (mData->isLoadable())
        {
            if ((u64)mData->data().size() < mData->fileSize())
            {
                // load map
                setState(WaveItem::Loading);
                startLoader(workdir, trans, sbar);
            }
            else
            {
                // generate from existing map
                deletePainted();
                if (!mData->data().isEmpty())
                {
                    WaveDataProviderMap wdp(mData->data());
                    wdp.setGroup(isGroup());
                    mPainted.generate(&wdp,trans,sbar,&mLoop);
                    setState(WaveItem::Painted);
                    if (mVisibleRange) Q_EMIT doneLoading();
                }
            }
        }
        else
        {
            // graphics from file
            setState(WaveItem::Loading);
            startLoader(workdir,trans, sbar);
        }
    }

    void WaveItem::startLoader(const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar)
    {
        Q_ASSERT(mState == Loading);
        mLoadValidity = WaveFormPaintValidity(trans,sbar);
        if (mLoader)
        {
            qDebug() << mData->fileIndex() << "*** warning *** : loader already running";
        }
        mLoader = new WaveLoaderThread(this, workdir, trans, sbar);
        connect(mLoader, &QThread::finished, this, &WaveItem::handleWaveLoaderFinished);
        mLoader->start();
    }

    void WaveItem::setState(State stat)
    {
        if (mState == stat) return;
 //       qDebug() << mFileIndex << "state change" << mState << "->" << stat;
        mState = stat;
        if (mState == Painted)
        {
            mLoadProgress = 0;
            mData->setDirty(false);
        }
        if (mState == Null)
            mLoadProgress = 0;
    }

    void WaveItem::incrementLoadProgress()
    {
        mLoadProgress += 5;
        if (mLoadProgress >= mLoadValidity.width())
            mLoadProgress = 5;
    }

    void WaveItem::dump(QTextStream &xout) const
    {
        if (isPainted())
            xout << mData->id() << mData->name() << mState << mData->data().size() << mPainted.numberPrimitives() << mPainted.x0() << mPainted.x1() << "\n";
        else
            xout << mData->id() << mData->name() << mState << mData->data().size() << "\n";

    }

    int WaveItem::cursorValue(float tCursor, int xpos)
    {
        // can deliver stored value
        int retval = mPainted.cursorValueStored(tCursor,xpos);
        if (retval != SaleaeDataTuple::sReadError) return retval;

        // get clock value
        if (mData->netType() == WaveData::ClockNet)
        {
            SimulationInput::Clock clk = static_cast<const WaveDataClock*>(mData)->clock();
            quint64 ntrans = floor(tCursor/clk.switch_time);
            retval = clk.start_at_zero ? ntrans % 2 : 1 - ntrans % 2;
            mPainted.setCursorValue(tCursor,xpos,retval);
            return retval;
        }

        // get value from memory map
        if (mData->isLoadable())
        {
            retval = mData->intValue(tCursor);
            mPainted.setCursorValue(tCursor,xpos,retval);
            return retval;
        }

        // try get from painted primitives, will store time
        retval = mPainted.cursorValuePainted(tCursor,xpos);
        /*
        if (retval == SaleaeDataTuple::sReadError && !mWorkdir.isEmpty())
        {
            bool canLoad = false;
            if (mMutex.tryLock())
            {
                if (!isLoading() && !isThreadBusy())
                {
                    setState(Loading);
                    canLoad = true;
                }
                mMutex.unlock();
            }

            // start value loader thread
            if (canLoad)
            {
                mLoader = new WaveValueThread(this,mWorkdir,tCursor,xpos);
                connect(mLoader,&QThread::finished,this,&WaveItem::handleValueLoaderFinished);
                mLoader->start();
            }
        }
        */
        return retval;
    }

    void WaveItem::abortLoader()
    {
        setState(Aborted);
        mLoop = false;
    }

    void WaveItem::handleWaveLoaderFinished()
    {
        mMutex.lock();
        if (isAborted())
        {
            setState(Null);
            deletePainted();
        }
        else if (isFinished())
        {
            setState(Painted);
        }
        else
            qDebug() << mData->fileIndex() << "invalid state when terminating thread" << mState << hex << (quintptr) mLoader << dec;
        mLoader->deleteLater();
        mLoader = nullptr;
        mMutex.unlock();
        if (mState == Painted && mVisibleRange)
            Q_EMIT doneLoading();
    }

    void WaveItem::loadSaleae(SaleaeInputFile& sif)
    {
        mData->loadSaleae(sif);
    }

//------------------------

    /** formerly paint handles request
    Q_UNUSED(option);
        Q_UNUSED(widget);

        if (hasRequest(DeleteAcknowledged))
            return;


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

        -----------------------
            if (false)  // TODO
            {
                if (mData->netType() == WaveData::ClockNet)
                    construct();
                else
                    prepareGeometryChange();
            }
            else if (mData && mData->bits() > 1)
            {
         ----------------------------
                if (sValuesAsText)
                {
                    if (!mGrpRects.isEmpty()) construct();
                }
                else
                {
                    if (!childItems().isEmpty()) construct();
                }
           }
           --------------------------
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
    */

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

    WaveItem *WaveItemHash::addOrReplace(WaveData*wd, WaveItemIndex::IndexType tp, int inx, int parentId)
    {
        WaveItemIndex wii(inx, tp, parentId);
        WaveItem* wi = value(wii);
        if (!wi)
        {
            wi = new WaveItem(wd);
            insert(wii,wi);
            wi->setRequest(WaveItem::AddRequest);
        }
        else
            wi->setWaveData(wd);
        return wi;
    }

    void WaveItemHash::dump(const char* stub)
    {
        // TODO
    }
}

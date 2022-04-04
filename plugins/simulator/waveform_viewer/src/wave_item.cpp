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

    const char* WaveItem::sBackgroundColor = "#0D293E" ;

    WaveItem::WaveItem(WaveData *dat, QObject *parent)
        : QObject(parent), mData(dat), mLoader(nullptr), mLoadProgress(0), mState(Null),
          mVisibleRange(false), mLoop(false),
          mYposition(-1), mRequest(0), mMinTime(0),
          mMaxTime(1000), mMaxTransition(0),
          mVisibile(true), mSelected(false)
    {;}

    WaveItem::~WaveItem()
    {
//        qDebug() << "delete wave item" << hex << (quintptr) this;
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

    void WaveItem::startGeneratePainted(const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar, const WaveDataTimeframe& tframe)
    {
        // precondition: mutex lock is set, state is Null
        mLoadProgress = 0;

        if (mData->loadPolicy() != WaveData::TooBigToLoad)
        {
            if ((u64)mData->data().size() < mData->fileSize())
            {
                // load map
                setState(WaveItem::Loading);
                startLoader(workdir, trans, sbar, tframe);
            }
            else
            {
                // generate from existing map
                deletePainted();
                if (!mData->data().isEmpty())
                {
                    WaveDataProviderMap wdp(mData->data());
                    wdp.setGroup(isGroup(),mData->bits(),mData->valueBase());
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
            startLoader(workdir,trans, sbar, tframe);
        }
    }

    void WaveItem::startLoader(const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar, const WaveDataTimeframe& tframe)
    {
        Q_ASSERT(mState == Loading);
        mLoadValidity = WaveZoomShift(trans,sbar);
        if (mLoader)
        {
            qDebug() << mData->fileIndex() << "*** warning *** : loader already running";
        }
        mLoader = new WaveLoaderThread(this, workdir, trans, sbar, tframe);
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

    int WaveItem::cursorValue(double tCursor, int xpos)
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
            Q_EMIT gotCursorValue();
            return retval;
        }

        // get value from memory map
        if (mData->loadPolicy() != WaveData::TooBigToLoad)
        {
            retval = mData->intValue(tCursor);
            mPainted.setCursorValue(tCursor,xpos,retval);
            Q_EMIT gotCursorValue();
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

    void WaveItem::loadSaleae()
    {
        mData->loadSaleae();
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


    bool WaveItemIndex::operator==(const WaveItemIndex &other) const
    {
        return mType == other.mType && mIndex == other.mIndex && mParentId == other.mParentId;
    }

    uint qHash(const WaveItemIndex& wii)
    {
        if (!wii.isValid()) return 0;
        return (wii.parentId() << 20) | ((wii.index()+1) << 2) | wii.intType();
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

    WaveItem* WaveItemHash::addOrReplace(WaveData*wd, WaveItemIndex::IndexType tp, int iwave, int parentId)
    {
        WaveItemIndex wii(iwave, tp, parentId);
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

    void WaveItemHash::dispose(WaveItem* wi)
    {
        if (!wi) return;
        wi->setRequest(WaveItem::DeleteRequest);
        mTrashCan.append(wi);
    }

    void WaveItemHash::emptyTrash()
    {
        auto it = mTrashCan.begin();
        while (it != mTrashCan.end())
        {
            WaveItem* wi = *it;
            switch (wi->state())
            {
            case WaveItem::Null:
            case WaveItem::Painted:
                wi->setRequest(WaveItem::DeleteAcknowledged);
                it = mTrashCan.erase(it);
                wi->deleteLater();
                break;
            case WaveItem::Loading:
                if (wi->hasLoader())
                    wi->abortLoader();
                ++it;
                break;
            default:
                ++it;
                break;
            }
        }
    }

    void WaveItemHash::dump(const char* stub)
    {
        Q_UNUSED(stub);
        // TODO
    }
}

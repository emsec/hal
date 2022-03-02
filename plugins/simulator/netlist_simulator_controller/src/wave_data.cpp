#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"
#include <math.h>
#include <vector>
#include <QString>
#include <QVector>
#include <QSet>
#include <QDir>
#include <stdio.h>
#include <QDebug>

namespace hal {

    WaveDataClock::WaveDataClock(const Net* n, const SimulationInput::Clock& clk, u64 tmax)
        : WaveData(n, WaveData::ClockNet), mClock(clk), mMaxTime(tmax)
    {
        dataFactory();
    }

    WaveDataClock::WaveDataClock(const Net* n, int start, u64 period, u64 tmax)
        : WaveData(n, WaveData::ClockNet), mMaxTime(tmax)
    {
        mClock.clock_net = n;
        mClock.switch_time = period / 2;
        mClock.start_at_zero = (start==0);
    }

    void WaveDataClock::setMaxTime(u64 tmax)
    {
        mMaxTime = tmax;
        resetWave();
        dataFactory();
    }

    void WaveDataClock::dataFactory()
    {
        int val = mClock.start_at_zero ? 0 : 1;
        for (u64 t=0; t<=mMaxTime; t+=mClock.switch_time)
        {
            mData.insert(t,val);
            val = val ? 0 : 1;
        }
    }

    WaveData::WaveData(const WaveData& other)
        : mId(other.mId), mFileIndex(other.mFileIndex),
          mName(other.mName), mNetType(other.mNetType), mBits(other.mBits), mValueBase(other.mValueBase),
          mData(other.mData), mDirty(true)
    {;}

    WaveData::WaveData(u32 id_, const QString& nam, NetType tp, const QMap<u64,int> &dat)
        : mId(id_), mFileIndex(-1), mName(nam), mNetType(tp), mBits(1), mValueBase(16), mData(dat), mDirty(true)
    {;}

    WaveData::WaveData(const Net* n, NetType tp)
        : mId(n->get_id()), mFileIndex(-1),
          mName(QString::fromStdString(n->get_name())),
          mNetType(tp), mBits(1), mValueBase(16), mDirty(true)
    {;}

    WaveData::~WaveData()
    {
    //    qDebug() << "~WaveData" << mId << mName;
    }

    void WaveData::resetWave()
    {
        mData.clear();
    }

    void WaveData::setId(u32 id_)
    {
        mId    = id_;
        mDirty = true;
    }

    bool WaveData::rename(const QString& nam)
    {
        if (mName == nam) return false;
        mName  = nam;
        mDirty = true;
        return true;
    }

    void WaveData::setBits(int bts)
    {
        mBits  = bts;
        mDirty = true;
    }


    void WaveData::insertBooleanValueWithoutSync(u64 t, BooleanFunction::Value bval)
    {
        int val = (int) bval;
        if (!mData.isEmpty())
        {
            auto it = mData.upperBound(t);
            if (it != mData.constBegin())
            {
                --it;
                if (it.value() == bval) return; // Nothing to do, previous value matches
            }
        }
        mData.insert(t,val);
        mDirty = true;
    }

    void WaveData::setData(const QMap<u64,int>& dat)
    {
        mData = dat;
        mDirty = true;
    }

    void WaveData::setStartvalue(int val)
    {
        for (auto it=mData.begin(); it!=mData.end(); ++it)
        {
            *it = val;
            val = val ? 0 : 1;
        }
    }

    int WaveData::get_value_at(u64 t) const
    {
        return intValue(t);
    }

    bool WaveData::isEqual(const WaveData& other, int tolerance) const
    {
        if (mData.size() != other.mData.size()) return false;
        auto jt = other.mData.constBegin();
        for (auto it = mData.begin(); it != mData.end(); ++it)
        {
            if (it.value() != jt.value()) return false;
            if (abs((int)(it.key()-jt.key())) > tolerance) return false;
            ++jt;
        }
        return true;
    }

    std::vector<std::pair<u64,int>> WaveData::get_events() const
    {
        std::vector<std::pair<u64,int>> retval;
        for (auto it = mData.constBegin(); it != mData.constEnd(); ++it)
            retval.push_back(std::make_pair(it.key(),it.value()));
        return retval;
    }


    bool WaveData::insertToggleTime(u64 t)
    {
        if (mData.isEmpty()) return false; // no start value
        auto it = mData.upperBound(t);
        if (it == mData.begin()) return false; // cannot insert before 0
        auto prevIt = it - 1;
        if (prevIt.key() == t)
            return false; // toggle time already exists
        int val = prevIt.value() ? 0 : 1;
        it = mData.insert(it, t, val);
        for (auto nextIt = it+1; nextIt != mData.end(); ++nextIt)
        {
            val = val ? 0 : 1;
            *nextIt = val;
        }
        return true;
    }

    bool WaveData::loadSaleae(const SaleaeDirectory& sd, const WaveDataTimeframe &tframe)
    {
        resetWave();
        std::filesystem::path path = sd.get_datafile(mName.toStdString(),mId);
        if (path.empty()) return false;

        SaleaeInputFile sif(path);
        loadSaleae(sif, tframe);
        return true;
    }

    void WaveData::loadSaleae(SaleaeInputFile& sif, const WaveDataTimeframe& tframe)
    {
        resetWave();
        SaleaeDataBuffer* sdb = sif.get_buffered_data();
        u64 n = sdb->mCount;
        if (!n)
        {
            mDirty = true;
            delete sdb;
            return;
        }
        u64 t0 = tframe.hasUserTimeframe() ? tframe.sceneMinTime() : 0;
        u64 t1 = tframe.hasUserTimeframe() ? tframe.sceneMaxTime() : sdb->mTimeArray[n-1];
        Q_ASSERT(t0 <= t1);
        int lastVal = BooleanFunction::X;
        bool valuePending = false;
        for (u64 i=0; i<n; i++)
        {
            u64 t = sdb->mTimeArray[i];
            if (t < t0)
            {
                lastVal = sdb->mValueArray[i];
                valuePending = true;
            }
            else if (t == t0)
            {
                mData.insert(t,sdb->mValueArray[i]);
                valuePending = false;
            }
            else if (t <= t1)
            {
                if (valuePending) mData.insert(t0,lastVal);
                mData.insert(t,sdb->mValueArray[i]);
                valuePending = false;
            }
            else
            {
                if (valuePending) mData.insert(t0,lastVal);
                valuePending = false;
                break;
            }
        }
        mDirty = true;
        delete sdb;
        return;
    }

    void WaveData::saveSaleae(SaleaeDirectory& sd)
    {
        SaleaeDirectoryStoreRequest save(&sd);
        mFileIndex = sd.get_datafile_index(mName.toStdString(),mId);
        if (mFileIndex < 0)
        {
            mFileIndex = sd.get_next_available_index();
            QDir saleaeDir(QString::fromStdString(sd.get_directory()));
            if (!saleaeDir.exists()) saleaeDir.mkpath(saleaeDir.absolutePath());
        }

        SaleaeDirectoryNetEntry sdne(mName.toStdString(),mId);
        sdne.addIndex(SaleaeDirectoryFileIndex(mFileIndex,0,maxTime(),mData.size()));
        sd.add_or_replace_net(sdne);

        std::filesystem::path path = sd.get_datafile(mName.toStdString(),mId);

        mFileSize = mData.size();

        SaleaeDataBuffer sdb(mFileSize);
        int j = 0;
        for (auto it = mData.constBegin(); it != mData.constEnd(); ++it)
        {
            sdb.mTimeArray[j] = it.key();
            sdb.mValueArray[j] = it.value();
            ++j;
        }
        SaleaeOutputFile sof(path.string(),mFileIndex);
        sof.put_data(&sdb);
    }

    QMap<u64,int>::const_iterator WaveData::timeIterator(float t) const
    {
        if (t<0) return mData.constEnd();
        QMap<u64,int>::const_iterator retval = mData.upperBound((u64)floor(t));
        if (retval != mData.constBegin()) --retval;
        return retval;
    }

    int WaveData::intValue(float t) const
    {
        if (mData.isEmpty()) return -1;
        QMap<u64,int>::const_iterator it = timeIterator(t);
        return it.value();
    }

    QString WaveData::strValue(float t) const
    {
        if (mData.isEmpty()) return "x";
        QMap<u64,int>::const_iterator it = timeIterator(t);
        return strValue(it);
    }

    QString WaveData::strValue(const QMap<u64,int>::const_iterator& it) const
    {
        if (it == mData.constEnd()) return QString();
        return strValue(it.value());
    }

    QString WaveData::strValue(int val) const
    {
        int bts = bits();
        switch (val) {
        case -2 : return "z";
        case -1 : return "x";
        }
        if (bts <= 1 || !val)
            return QString::number(val);
        if (mValueBase<0)
        {
            int mask = 1 << (bts-1);
            if (val&mask)
                return QString("-%1").arg((1 << bts) - val);
            else
                return QString::number(val);
        }
        int nDigits = 0;
        switch (mValueBase)
        {
        case 2:
            nDigits = bits();
            return QString("0b%1").arg((uint)val,nDigits,2,QLatin1Char('0'));
        case 16:
            nDigits = bits() / 4;
            return QString("0x%1").arg((uint)val,nDigits,16,QLatin1Char('0'));
        default: break;
        }
        return QString::number(val,mValueBase);
    }

    u64 WaveData::maxTime() const
    {
        if (mData.isEmpty()) return 0;
        return mData.lastKey();
    }

    void WaveData::eraseAtTime(u64 t)
    {
        auto it = mData.find(t);
        if (it == mData.end()) return;
        int val = it.value();
        it = mData.erase(it);
        if (mBits <= 1)
            while (it != mData.end())
            {
                int nextVal = it.value();
                *(it++) = val;
                val = nextVal;
            }
    }

//--------------------------------------------
    WaveDataGroupIndex::WaveDataGroupIndex(const WaveData* wd)
    {
        if (!wd->id()) mCode = 0;
        else
        {
            const WaveDataGroup* grp = dynamic_cast<const WaveDataGroup*>(wd);
            if (grp) construct (grp->id(), false);
            else     construct (wd->id(),  true);
        }
    }

    void WaveDataGroupIndex::construct(u32 id, bool isNet)
    {
        if (!id) mCode = 0;
        else
            mCode = (id << 1) | (isNet?0:1);
    }

    uint qHash(const WaveDataGroupIndex& wdgi) { return wdgi.mCode; }

//--------------------------------------------
    u32 WaveDataGroup::sMaxGroupId = 0;

    WaveDataGroup::WaveDataGroup(WaveDataList *wdList, int grpId, const QString& nam)
        : WaveData(grpId,nam,WaveData::NetGroup), mWaveDataList(wdList)
    {
        mWaveDataList->registerGroup(this);
    }

    WaveDataGroup::WaveDataGroup(WaveDataList* wdList, const QString& nam)
        : WaveData(++sMaxGroupId,nam,WaveData::NetGroup), mWaveDataList(wdList)
    {
        if (nam.isEmpty()) rename(QString("group_%1").arg(id()));
        mWaveDataList->registerGroup(this);
    }

    WaveDataGroup::WaveDataGroup(WaveDataList* wdList, const WaveData* wdGrp)
        : WaveData(++sMaxGroupId,wdGrp->name(),WaveData::NetGroup), mWaveDataList(wdList)
    {
        int n = wdGrp->bits();
        if (!n) return;

        QMap<u64,int>** bitValue = new QMap<u64,int>*[n];
        for (int i=0; i<n; i++) bitValue[i] = new QMap<u64,int>();

        bool first = true;
        int lastV = 0;
        for (auto it = wdGrp->data().constBegin(); it != wdGrp->data().constEnd(); ++it)
        {
            u64 t = it.key();
            int v = it.value();
            for (int i=0; i<n ; i++)
            {
                int mask = 1 << i;
                if ( (v&mask) != (lastV&mask) || first )
                    bitValue[i]->insert(t, (v&mask) ? 1 : 0);
            }
            first = false;
            lastV = v;
        }
        for (int i=0; i<n; i++)
        {
            // Create Fake Wave Entries
            WaveData* wd = new WaveData(id()*10000+i, QString("%1_bit%2").arg(name()).arg(i), WaveData::RegularNet, *bitValue[i]);
            wd->setBits(1);
            mGroupList.append(wd);
            mIndex.insert(WaveDataGroupIndex(wd),i);
            mWaveDataList->add(wd,false);
            delete bitValue[i];
        }
        delete [] bitValue;
        mWaveDataList->registerGroup(this);
    }

    WaveDataGroup::~WaveDataGroup()
    {
        auto it = mWaveDataList->mDataGroups.find(id());
        if (it != mWaveDataList->mDataGroups.end())
            mWaveDataList->mDataGroups.erase(it);
    }

    void WaveDataGroup::replaceChild(WaveData* wd)
    {
        int inx = childIndex(wd);
        if (inx<0) return;
        mGroupList[inx] = wd;
        recalcData();
    }

    void WaveDataGroup::restoreIndex()
    {
        SaleaeDirectoryStoreRequest save(&mWaveDataList->saleaeDirectory());
        SaleaeDirectoryGroupEntry* sdge = mWaveDataList->saleaeDirectory().get_group(id());
        if (sdge) sdge->get_nets().clear();

        mIndex.clear();
        int inx = 0;
        for (const WaveData* wd : mGroupList)
        {
            if (sdge)
                sdge->add_net(SaleaeDirectoryNetEntry(wd->name().toStdString(),wd->id()));
            mIndex[WaveDataGroupIndex(wd)] = inx++;
        }
    }

    void WaveDataGroup::addNet(const Net* n)
    {
        if (bits() >= 30) return;
        u32 netId = n->get_id();
        WaveData* wd = nullptr;
        if (!mWaveDataList->hasNet(netId))
            mWaveDataList->add((wd = new WaveData(n)),false);
        else
            wd = mWaveDataList->waveDataByNet(n);
        int inx = mGroupList.size();
        mGroupList.append(wd);
        mIndex.insert(WaveDataGroupIndex(wd),inx);
    }

    QList<WaveData*> WaveDataGroup::children() const
    {
        return mGroupList;
    }

    QList<int> WaveDataGroup::childrenWaveIndex() const
    {
        QList<int> retval;
        for (const WaveData* wd : mGroupList)
        {
            retval.append(mWaveDataList->waveIndexByNetId(wd->id()));
        }
        return retval;
    }

    WaveData* WaveDataGroup::childAt(int inx) const
    {
        if (inx >= mGroupList.size()) return nullptr;
        return mGroupList.at(inx);
    }

    int WaveDataGroup::childIndex(WaveData* wd) const
    {
        return mIndex.value(WaveDataGroupIndex(wd),-1);
    }

    int WaveDataGroup::netIndex(u32 id) const
    {
        return mIndex.value(WaveDataGroupIndex(id,true),-1);
    }

    bool WaveDataGroup::hasNetId(u32 id) const
    {
        return mIndex.contains(WaveDataGroupIndex(id,true));
    }

    int WaveDataGroup::bits() const
    {
        int n = mGroupList.size();
        if (n < 1) n=1;
        return n;
    }

    WaveData* WaveDataGroup::removeAt(int inx)
    {
        if (inx >= mGroupList.size()) return nullptr;
        WaveData* wd = mGroupList.at(inx);
        mGroupList.removeAt(inx);
        restoreIndex();
        recalcData();
        return wd;
    }

    void WaveDataGroup::insert(int inx, WaveData* wd)
    {
        if (inx >= mGroupList.size())
            mGroupList.append(wd);
        else
            mGroupList.insert(inx,wd);
        restoreIndex();
        recalcData();
    }

    void WaveDataGroup::addWaves(const QVector<WaveData*>& wds)
    {
        mGroupList.append(wds.toList());
        restoreIndex();
        recalcData();
    }

    void WaveDataGroup::updateWaveData(WaveData* wd)
    {
        int inx = childIndex(wd);
        if (inx < 0) return;
        mGroupList[inx] = wd;
        recalcData();
    }

    bool WaveDataGroup::isLoadable() const
    {
        for (const WaveData* wd : mGroupList)
            if (!wd->isLoadable()) return false;
        return true;
    }

    void WaveDataGroup::recalcData()
    {
        mData.clear();
        QMultiMap<u64,int> tIndex;
        u32 value = 0;
        u32 undef = 0;
        int nChildren = mGroupList.size();
        WaveData** wdArray = new WaveData*[nChildren];
        if (mGroupList.isEmpty())
        {
            mData[0] = -1;
            mDirty = true;
            return;
        }
        for (int ibit = 0; ibit < nChildren; ibit++)
        {
            undef |= (1 << ibit);
            WaveData* wd = mGroupList.at(ibit);
            if (wd->isLoadable() && wd->data().size() < (int) wd->fileSize())
                wd->loadSaleae(mWaveDataList->saleaeDirectory(),mWaveDataList->timeFrame());
            wdArray[ibit] = wd;
            if (!wd) continue;
            for (u64 t : wd->data().keys())
                tIndex.insert(t,ibit);
        }
        u64 t0 = 0;
        for (auto it = tIndex.constBegin(); it != tIndex.constEnd(); ++it)
        {
            if (it.key() != t0)
            {
                mData.insert(t0, undef ? -1 : value);
                t0 = it.key();
            }
            int ibit = it.value();
            int v = wdArray[ibit]->data().value(t0);
            int mask = (1 << ibit);
            if (v<0)
                undef |= mask;
            else
            {
                undef &= ( ~mask );
                if (v)
                    value |= mask;
                else
                    value &= ( ~mask );
            }
        }
        mData.insert(t0, undef ? -1 : value);
        delete [] wdArray;
        mDirty = true;
        mWaveDataList->emitGroupUpdated(id());
    }

//--------------------------------------------
    WaveDataTimeframe::WaveDataTimeframe()
            : mSceneMaxTime(sMinSceneWidth), mSimulateMaxTime(0), mUserdefMaxTime(0), mUserdefMinTime(0) {;}

    u64 WaveDataTimeframe::simulateMaxTime() const
    {
        return mSimulateMaxTime;
    }

    u64 WaveDataTimeframe::sceneMaxTime() const
    {
        if (hasUserTimeframe()) return mUserdefMaxTime;
        return mSceneMaxTime;
    }

    void WaveDataTimeframe::setSceneMaxTime(u64 t)
    {
        if (t < sMinSceneWidth)
            mSceneMaxTime = sMinSceneWidth;
        else
            mSceneMaxTime = t;
    }

    u64 WaveDataTimeframe::sceneMinTime() const
    {
        if (hasUserTimeframe()) return mUserdefMinTime;
        return 0;
    }

    u64 WaveDataTimeframe::sceneWidth() const
    {
        u64 x0 = sceneMinTime();
        u64 x1 = sceneMaxTime();
        if (x1 <= x0) return 1;
        return (x1-x0);
    }

    bool WaveDataTimeframe::hasUserTimeframe() const
    {
        return mUserdefMaxTime > 0;
    }

    void WaveDataTimeframe::setUserTimeframe(u64 t0, u64 t1)
    {
        mUserdefMinTime = t0;
        mUserdefMaxTime = t1;
    }

//--------------------------------------------
    WaveDataList::WaveDataList(const QString& sdFilename, QObject* parent)
        : QObject(parent),
          mSaleaeDirectory(sdFilename.toStdString())
    {;}

    WaveDataList::~WaveDataList()
    {
        clearAll();
    }

    void WaveDataList::setMaxTime(u64 tmax)
    {
        if (mTimeframe.mSceneMaxTime == tmax) return;

        // adjust clock settings
        bool mustUpdateClocks = (tmax > mTimeframe.mSceneMaxTime);

        mTimeframe.setSceneMaxTime(tmax);
        if (mustUpdateClocks) updateClocks();
        Q_EMIT timeframeChanged(&mTimeframe);
    }

    void WaveDataList::incrementSimulTime(u64 deltaT)
    {
        mTimeframe.mSimulateMaxTime += deltaT;
        if (mTimeframe.mSimulateMaxTime > mTimeframe.mSceneMaxTime)
            setMaxTime(mTimeframe.mSimulateMaxTime);
    }
    
    QList<const WaveData*> WaveDataList::toList() const
    {
        QList<const WaveData*> retval;
        for (const WaveData* wd : *this)
            retval.append(wd);
        return retval;
    }

    QList<const WaveData*> WaveDataList::partialList(u64 start_time, u64 end_time, std::set<const Net*> &nets) const
    {
        QSet<u32> netIds;
        for (const Net* n : nets)
            netIds.insert(n->get_id());

        QList<const WaveData*> retval;

        for (const WaveData* wd : *this)
        {
            if (!netIds.isEmpty() && !netIds.contains(wd->id())) continue;
            QMap<u64,int> xdata = wd->data();
            auto it = xdata.begin();
            int rememberValue = -99;
            while (it != xdata.end())
            {
                if (it.key() < start_time)
                {
                    rememberValue = it.value();
                    it = xdata.erase(it);
                }
                else if (it.key() == start_time)
                {
                    rememberValue = -99;
                    ++it;
                }
                else if (end_time && it.key() > end_time)
                    it = xdata.erase(it);
                else
                {
                    ++it;
                }
            }
            if (rememberValue > -99)
                xdata.insert(start_time,rememberValue);
            WaveData* wdCopy = new WaveData(wd->id(),wd->name(),wd->netType(),xdata);
            retval.append(wdCopy);
        }
        return retval;
    }

    void WaveDataList::setUserTimeframe(u64 t0, u64 t1)
    {
        if (t0 == mTimeframe.mUserdefMinTime && t1 == mTimeframe.mUserdefMaxTime) return;
        mTimeframe.setUserTimeframe(t0,t1);
        Q_EMIT timeframeChanged(&mTimeframe);
    }

    void WaveDataList::clearAll()
    {
        bool notEmpty = ! isEmpty();
        mIds.clear();
        if (notEmpty)
            Q_EMIT waveRemoved(-1);
        setMaxTime(0);
        for (auto it=begin(); it!=end(); ++it)
            delete *it;
        clear();
    }

    void WaveDataList::dump() const
    {
        fprintf(stderr, "WaveDataList:_________%8u______________\n", (unsigned int) mTimeframe.mSimulateMaxTime);
        for (auto it = constBegin(); it!= constEnd(); ++it)
        {
            fprintf(stderr, "  %4d <%s>:", (*it)->id(), (*it)->name().toStdString().c_str());
            for (auto jt = (*it)->data().begin(); jt != (*it)->data().end(); ++jt)
                fprintf(stderr, " <%u,%d>", (unsigned int) jt.key(), jt.value());
            fprintf(stderr, "\n");
        }
        fflush(stderr);

    }

    void WaveDataList::updateWaveName(int inx, const QString& nam)
    {
        if (at(inx)->rename(nam))
        {
            SaleaeDirectoryStoreRequest save(&mSaleaeDirectory);
            Q_EMIT nameUpdated(inx);
            mSaleaeDirectory.rename_net(at(inx)->id(),nam.toStdString());
        }
    }

    void WaveDataList::emitWaveAdded(int inx)
    {
        Q_EMIT waveAdded(inx);
    }

    void WaveDataList::emitWaveUpdated(int inx)
    {
        u32 netId = at(inx)->id();
        for (auto it = mDataGroups.begin(); it != mDataGroups.end(); ++it)
            if (it.value()->hasNetId(netId))
            {
                Q_EMIT waveUpdated(inx,it.value()->id());
            }
        Q_EMIT waveUpdated(inx,0);
    }

    void WaveDataList::emitGroupUpdated(int grpId)
    {
        Q_EMIT groupUpdated(grpId);
    }

    void WaveDataList::updateWaveData(int inx)
    {
        WaveData* wd = at(inx);
        wd->saveSaleae(mSaleaeDirectory);
        u32 netId = wd->id();
        for (auto it = mDataGroups.begin(); it != mDataGroups.end(); ++it)
            if (it.value()->hasNetId(netId))
            {
                it.value()->recalcData();
                Q_EMIT waveUpdated(inx,it.value()->id());
            }
        Q_EMIT waveUpdated(inx,0);
    }

    void WaveDataList::updateClocks()
    {
        for (auto it=begin(); it!=end(); ++it)
            if ((*it)->netType() == WaveData::ClockNet)
            {
                WaveDataClock* wdc = static_cast<WaveDataClock*>(*it);
                wdc->setMaxTime(mTimeframe.mSceneMaxTime);
            }
    }

    void WaveDataList::updateMaxTime()
    {
        u64 tmax = 0;
        for (auto it = constBegin(); it!= constEnd(); ++it)
        {
            u64 tmaxWave = (*it)->maxTime();
            if (tmaxWave > tmax) tmax = tmaxWave;
        }
        setMaxTime(tmax);
    }

    void WaveDataList::add(WaveData* wd, bool updateSaleae)
    {
        int n = size();

        mIds[wd->id()] = n;
        append(wd);
        updateMaxTime();
        if (updateSaleae) wd->saveSaleae(mSaleaeDirectory);
//        if (!silent) Q_EMIT waveAdded(n);
        testDoubleCount();
    }

    void WaveDataList::triggerAddToView(u32 id) const
    {
        int iwave = waveIndexByNetId(id);
        if (iwave<0) return;
        Q_EMIT waveAdded(iwave);
    }

    void WaveDataList::registerGroup(WaveDataGroup *grp)
    {
        u32 grpId = grp->id();
        Q_ASSERT(!mDataGroups.contains(grpId));
        mDataGroups.insert(grpId,grp);
        if (grpId)
        {
            mSaleaeDirectory.add_group(SaleaeDirectoryGroupEntry(grp->name().toStdString(),grpId));
            updateMaxTime();
            Q_EMIT groupAdded(grp->id());
        }
    }

    u32 WaveDataList::createGroup(QString grpName)
    {
        WaveDataGroup* grp = new WaveDataGroup(this, grpName);
        return grp->id();
    }

    void WaveDataList::addWavesToGroup(u32 grpId, const QVector<WaveData*>& wds)
    {
        SaleaeDirectoryStoreRequest save(&mSaleaeDirectory);
        WaveDataGroup* grp = mDataGroups.value(grpId);
        QVector<u32> netIds;
        netIds.reserve(wds.size());
        if (!grp) return;
        int inx = grp->size();
        for (WaveData* wd : wds)
        {
            netIds.append(wd->id());
            grp->insert(inx++,wd);
        }
        grp->restoreIndex();
        grp->recalcData();
        Q_EMIT waveAddedToGroup(netIds,grpId);
    }

    void WaveDataList::insertBooleanValue(WaveData *wd, u64 t, BooleanFunction::Value bval)
    {
        wd->insertBooleanValueWithoutSync(t, bval);
        wd->saveSaleae(mSaleaeDirectory);
    }

    void WaveDataList::removeGroup(u32 grpId)
    {
        WaveDataGroup* grp = mDataGroups.value(grpId);
        if (!grp) return;
        Q_EMIT groupAboutToBeRemoved(grp);
        mSaleaeDirectory.remove_group(grpId);
        delete grp;
    }

    void WaveDataList::replaceWaveData(int inx, WaveData* wdNew)
    {
        // replace existing
        triggerBeginResetModel();
        WaveData* wdOld = at(inx);
        Q_ASSERT(wdOld);
        Q_ASSERT(wdOld != wdNew);
        wdNew->rename(wdOld->name()); // TODO : update saleae ?
        operator[](inx) = wdNew;
        for (WaveDataGroup* grp : mDataGroups.values())
            if (grp->hasNetId(wdNew->id()))
                grp->replaceChild(wdNew);
        if (wdNew->maxTime() > mTimeframe.mSceneMaxTime)
            updateMaxTime();

        wdNew->saveSaleae(mSaleaeDirectory);


        emitWaveUpdated(inx);
        delete wdOld;
        triggerEndResetModel();
    }

    void WaveDataList::updateFromSaleae()
    {
        mSaleaeDirectory.parse_json();
        u64 sdMaxTime = mSaleaeDirectory.get_max_time();
        if (sdMaxTime > mTimeframe.mSimulateMaxTime) incrementSimulTime(sdMaxTime-mTimeframe.mSimulateMaxTime);

        // create empty WaveData instances for all SALEAE waves ...
        QMap<QString, WaveData*> saleaeWaves;
        for (const SaleaeDirectory::ListEntry& sdle : mSaleaeDirectory.get_net_list())
        {
            QString name = QString::fromStdString(sdle.name);
            WaveData* wd = new WaveData(sdle.id, name);
            wd->setFileIndex(sdle.fileIndex);
            wd->setFileSize(sdle.size);
            saleaeWaves.insert(name,wd);
        }

        // ... but delete waves if already existing in container
        for (int i=0; i<size(); i++)
        {
            WaveData* wd = at(i);
            auto it = saleaeWaves.find(wd->name());
            if (it == saleaeWaves.end()) continue;
            wd->setFileSize(it.value()->fileSize());
            wd->setFileIndex(it.value()->fileIndex());
            if (wd->isLoadable())
                wd->loadSaleae(mSaleaeDirectory,mTimeframe);
            emitWaveUpdated(i);
            delete it.value();
            saleaeWaves.erase(it);
        }

        for (auto it=saleaeWaves.begin(); it!=saleaeWaves.end(); ++it)
        {
            add(it.value(),false);
        }
        setMaxTime(mSaleaeDirectory.get_max_time());
    }

    void WaveDataList::addOrReplace(WaveData* wd)
    {
        Q_ASSERT(wd);
        int inx = mIds.value(wd->id(),-1);
        if (inx >= 0)
            replaceWaveData(inx, wd);
        else
            add(wd,true);
    }

    WaveData* WaveDataList::waveDataByNet(const Net *n)
    {
        if (!n) return nullptr;
        int inx = mIds.value(n->get_id(),-1);
        if (inx >= 0) return at(inx);
        WaveData* wd = new WaveData(n);
        if (wd->loadSaleae(mSaleaeDirectory,mTimeframe))
        {
            add(wd,false);
            return wd;
        }
        delete wd;
        return nullptr;
    }

    void WaveDataList::testDoubleCount()
    {
        QMap<u32,int> doubleCount;
        for (const WaveData* wd : *this)
           ++doubleCount[wd->id()];
        for (auto it=doubleCount.constBegin(); it!=doubleCount.constEnd(); ++it)
        {
            if (it.value() > 1)
            {
                qDebug() << "duplicate net" << it.value() << at(mIds.value(it.key()))->name();
            }
        }
    }

    void WaveDataList::restoreIndex()
    {
        mIds.clear();
        int inx = 0;

        for (const WaveData* wd : *this)
            mIds[wd->id()] = inx++;
        testDoubleCount();
    }

    QSet<u32> WaveDataList::toSet() const
    {
        QList<u32> keyList = mIds.keys();
        return keyList.toSet();
        // return QSet<u32>(keyList.begin(),keyList.end());
    }

    void WaveDataList::remove(u32 id)
    {
        auto it = mIds.find(id);
        if (it == mIds.end()) return;
        int inx = it.value();
        WaveData* toDelete = at(inx);
        removeAt(inx);
        restoreIndex();
        Q_EMIT waveRemoved(inx);
        delete toDelete;
    }

    void WaveDataList::setValueForEmpty(int val)
    {
        int inx = 0;

        for (auto it = constBegin(); it!= constEnd(); ++it)
        {
            if ((*it)->data().isEmpty())
            {
                (*it)->insert(0,val);
                emitWaveUpdated(inx);
            }
            ++inx;
        }
    }

}

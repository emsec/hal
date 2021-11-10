#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/net.h"
#include <math.h>
#include <vector>
#include <QString>
#include <stdio.h>

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
        clear();
        dataFactory();
    }

    void WaveDataClock::dataFactory()
    {
        int val = mClock.start_at_zero ? 0 : 1;
        for (u64 t=0; t<=mMaxTime; t+=mClock.switch_time)
        {
            insert(t,val);
            val = val ? 0 : 1;
        }
    }

    WaveData::WaveData(u32 id_, const QString& nam, NetType tp, const QMap<u64,int> &other)
        : QMap<u64,int>(other), mId(id_), mName(nam), mNetType(tp)
    {
        // qDebug() << "A:WaveData" << mId << mName << hex << (quintptr) this;
    }

    WaveData::WaveData(const Net* n, NetType tp)
        : mId(n->get_id()),
          mName(QString::fromStdString(n->get_name())),
          mNetType(tp), mBits(1)
    {
       // qDebug() << "B:WaveData" << mId << mName << hex << (quintptr) this;
    }

    WaveData::WaveData(u32 id_, const QString& nam, const QList<WaveData*>& wdList, NetType tp)
        : mId(id_),
          mName(nam),
          mNetType(tp), mBits(wdList.size())
    {
        if (mBits < 1 || mBits > 30) return;
        QMultiMap<u64,int> tIndex;
        u32 value = 0;
        u32 undef = 0;
        for (int ibit = 0; ibit < mBits; ibit++)
        {
            undef |= (1 << ibit);
            WaveData* wd = wdList.at(ibit);
            for (u64 t : wd->keys())
                tIndex.insert(t,ibit);
        }
        u64 t0 = 0;
        for (auto it = tIndex.constBegin(); it != tIndex.constEnd(); ++it)
        {
            if (it.key() != t0)
            {
                insert(t0, undef ? -1 : value);
                t0 = it.key();
            }
            int ibit = it.value();
            int v = wdList.at(ibit)->value(t0);
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
        insert(t0, undef ? -1 : value);
    }

    WaveData::~WaveData()
    {
        /*
        if (mId > 1000000)
            qDebug() << "==========";
        else
            qDebug() << "X:WaveData" << mId << mName << hex << (quintptr) this;
            */
    }

    void WaveData::insertBooleanValue(u64 t, BooleanFunction::Value bval)
    {
        int val = -1;
        switch (bval)
        {
        case BooleanFunction::Value::Z:    val = -2; break;
        case BooleanFunction::Value::X:    val = -1; break;
        case BooleanFunction::Value::ZERO: val =  0; break;
        case BooleanFunction::Value::ONE:  val =  1; break;
        }
        insert(t,val);
    }

    void WaveData::setStartvalue(int val)
    {
        for (auto it=begin(); it!=end(); ++it)
        {
            *it = val;
            val = val ? 0 : 1;
        }
    }

    bool WaveData::insertToggleTime(u64 t)
    {
        if (isEmpty()) return false; // no start value
        auto it = upperBound(t);
        if (it == begin()) return false; // cannot insert before 0
        auto prevIt = it - 1;
        if (prevIt.key() == t)
            return false; // toggle time already exists
        int val = prevIt.value() ? 0 : 1;
        it = insert(it, t, val);
        for (auto nextIt = it+1; nextIt != end(); ++nextIt)
        {
            val = val ? 0 : 1;
            *nextIt = val;
        }
        return true;
    }

    QMap<u64,int>::const_iterator WaveData::timeIterator(float t) const
    {
        if (t<0) return constEnd();
        QMap<u64,int>::const_iterator retval = upperBound((u64)floor(t));
        if (retval != constBegin()) --retval;
        return retval;
    }

    int WaveData::intValue(float t) const
    {
        QMap<u64,int>::const_iterator it = timeIterator(t);
        return it.value();
    }

    QString WaveData::strValue(float t) const
    {
        QMap<u64,int>::const_iterator it = timeIterator(t);
        return strValue(it);
    }

    QString WaveData::strValue(const QMap<u64,int>::const_iterator& it) const
    {
        if (it == constEnd()) return QString();
        switch (it.value()) {
        case -2 : return "z";
        case -1 : return "x";
        }
        return QString::number(it.value());
    }

    u64 WaveData::maxTime() const
    {
        if (isEmpty()) return 0;
        return lastKey();
    }

//--------------------------------------------

    WaveDataList::~WaveDataList()
    {
        clearAll();
    }

    void WaveDataList::setMaxTime(u64 tmax)
    {
        if (mMaxTime == tmax) return;

        // adjust clock settings
        bool mustUpdateClocks = (tmax > mMaxTime);

        mMaxTime = tmax;
        if (mustUpdateClocks) updateClocks();
        Q_EMIT maxTimeChanged(mMaxTime);
    }

    void WaveDataList::incrementMaxTime(u64 deltaT)
    {
        setMaxTime(mMaxTime + deltaT);
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
            auto it = wd->constBegin();
            if (it.key() < start_time) it = wd->lowerBound(start_time);
            if (it == wd->constEnd()) continue;
            WaveData* wdCopy = new WaveData(*wd);
            if (it.key() > start_time)
                wdCopy->insert(start_time,wd->intValue(start_time));
            while (it != wd->constEnd())
            {
                if (end_time && it.key() > end_time) break;
                wdCopy->insert(it.key(),it.value());
                ++it;
            }
            retval.append(wdCopy);
        }
        return retval;
    }

    void WaveDataList::clearAll()
    {
        bool notEmpty = ! isEmpty();
        clear();
        mIds.clear();
        if (notEmpty)
            Q_EMIT waveRemoved(-1);
        setMaxTime(0);
        for (auto it=begin(); it!=end(); ++it)
            delete *it;
    }

    void WaveDataList::dump() const
    {
        fprintf(stderr, "WaveDataList:_________%8u______________\n", (unsigned int) mMaxTime);
        for (auto it = constBegin(); it!= constEnd(); ++it)
        {
            fprintf(stderr, "  %4d <%s>:", (*it)->id(), (*it)->name().toStdString().c_str());
            for (auto jt = (*it)->begin(); jt != (*it)->end(); ++jt)
                fprintf(stderr, " <%u,%d>", (unsigned int) jt.key(), jt.value());
            fprintf(stderr, "\n");
        }
        fflush(stderr);

    }

    void WaveDataList::updateClocks()
    {
        for (auto it=begin(); it!=end(); ++it)
            if ((*it)->netType() == WaveData::ClockNet)
            {
                WaveDataClock* wdc = static_cast<WaveDataClock*>(*it);
                wdc->setMaxTime(mMaxTime);
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

    void WaveDataList::add(WaveData* wd)
    {
        int n = size();
        mIds[wd->id()] = n;
        append(wd);
        updateMaxTime();
        Q_EMIT waveAdded(n);
    }

    void WaveDataList::replaceWave(int inx, WaveData* wd)
    {
        // replace existing
        WaveData* wdDelete = at(inx);
        operator[](inx) = wd;
        if (wd->maxTime() > mMaxTime)
            updateMaxTime();
        Q_EMIT waveReplaced(inx);
        delete wdDelete;
    }


    void WaveDataList::addOrReplace(WaveData* wd)
    {
        Q_ASSERT(wd);
        int inx = mIds.value(wd->id(),-1);
        if (inx >= 0)
            replaceWave(inx, wd);
        else
            add(wd);
    }

    WaveData* WaveDataList::waveDataByNetId(u32 id) const
    {
        int inx = mIds.value(id,-1);
        if (inx < 0) return nullptr;
        return at(inx);
    }

    void WaveDataList::restoreIndex()
    {
        mIds.clear();
        int inx = 0;
        for (const WaveData* wd : *this)
        {
            mIds[wd->id()] = inx++;
        }
    }

    QSet<u32> WaveDataList::toSet() const
    {
        QList<u32> keyList = mIds.keys();
        // Qt > 5.15       return QSet<u32>(keyList.begin(),keyList.end());
        return keyList.toSet();
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
            if ((*it)->isEmpty())
            {
                (*it)->insert(0,val);
                Q_EMIT waveUpdated(inx);
            }
            ++inx;
        }
    }

}

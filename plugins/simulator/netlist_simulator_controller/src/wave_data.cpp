#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/net.h"
#include <math.h>
#include <vector>
#include <QString>
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
          mName(QString("%1[%2]")
                .arg(QString::fromStdString(n->get_name()))
                .arg(n->get_id())),
          mNetType(tp)
    {
       // qDebug() << "B:WaveData" << mId << mName << hex << (quintptr) this;
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

    /* TODO transfer results from netlist_simulator

    WaveData* WaveData::simulationResultFactory(Net *n, const NetlistSimulator* sim)
    {
        const std::vector<WaveEvent>& evts = sim->get_simulation_events(n);
        if (evts.empty()) return nullptr;
        WaveData* retval = new WaveData(n);
        for (const Event& evt : evts)
        {
        insertBooleanValue(evt.time,evt.new_value)
        }
        // no start value defined?
        auto it = retval->lowerBound(0);
        if (it == retval->end() || it.key() > 0)
            retval->insert(0,-1);
        return retval;
    }

    */

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

    char WaveData::charValue(float t) const
    {
        QMap<u64,int>::const_iterator it = timeIterator(t);
        return charValue(it);
    }

    char WaveData::charValue(const QMap<u64,int>::const_iterator& it) const
    {
        if (it == constEnd()) return '\0';
        switch (it.value()) {
        case -2 : return 'z';
        case -1 : return 'x';
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            return (char) ('0' + it.value());
        }
        return '\0';
    }

    WaveDataList::~WaveDataList()
    {
        clearAll();
    }

    void WaveDataList::incrementMaxTime(u64 deltaT)
    {
        mMaxTime += deltaT;
        Q_EMIT maxTimeChanged(mMaxTime);
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
        mMaxTime = 0;
        if (notEmpty)
            Q_EMIT waveRemoved(-1);
        Q_EMIT maxTimeChanged(0);
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
        for (auto it = constBegin(); it!= constEnd(); ++it)
        {
            if ((*it)->isEmpty()) continue;
            u64 maxT = (*it)->lastKey();
            if (maxT > mMaxTime)
            {
                mMaxTime = maxT;
                Q_EMIT maxTimeChanged(mMaxTime);
            }
        }
    }

    void WaveDataList::add(WaveData* wd)
    {
        int n = size();
        mIds[wd->id()] = n;
        append(wd);
        updateMaxTime();
        Q_EMIT waveAdded();
    }

    void WaveDataList::addOrReplace(WaveData* wd)
    {
        Q_ASSERT(wd);
        int inx = mIds.value(wd->id(),-1);
        if (inx >= 0)
        {
            // replace existing
            WaveData* toDelete = at(inx);
            operator[](inx) = wd;
            updateMaxTime();
            Q_EMIT waveReplaced(inx);
            delete toDelete;
        }
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

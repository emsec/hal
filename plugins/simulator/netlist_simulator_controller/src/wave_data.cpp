#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/net.h"
#include <math.h>
#include <vector>
#include <QString>


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
    {;}

    WaveData::WaveData(const Net* n, NetType tp)
        : mId(n->get_id()),
          mName(QString("%1[%2]")
                .arg(QString::fromStdString(n->get_name()))
                .arg(n->get_id())),
          mNetType(tp)
    {;}

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

    int WaveData::tValue(float t) const
    {
        if (t<0) return 0;
        auto it = upperBound((int)floor(t));
        if (it != constBegin()) --it;
        return it.value();
    }

    QString WaveData::textValue(const QMap<u64,int>::const_iterator& it) const
    {
        if (it == constEnd()) return QString();
        switch (it.value()) {
        case -2 : return "z";
        case -1 : return "x";
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            return QString() + (char) ('0' + it.value());
        }
        return QString();
    }

    WaveDataList::~WaveDataList()
    {
        clearAll();
    }

    void WaveDataList::incrementMaxTime(u64 deltaT)
    {
        mMaxTime += deltaT;
    }

    void WaveDataList::clearAll()
    {
        for (auto it=begin(); it!=end(); ++it)
            delete *it;
        clear();
        mIds.clear();
        mMaxTime = 0;
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
                mMaxTime = maxT;
        }
    }

    void WaveDataList::add(WaveData* wd)
    {
        int n = size();
        mIds[wd->id()] = n;
        append(wd);
        updateMaxTime();
    }

    void WaveDataList::addOrReplace(WaveData* wd)
    {
        Q_ASSERT(wd);
        int inx = mIds.value(wd->id(),-1);
        if (inx >= 0)
        {
            // replace existing
            delete at(inx);
            operator[](inx) = wd;
            updateMaxTime();
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
        delete at(inx);
        removeAt(inx);
        restoreIndex();
    }
}

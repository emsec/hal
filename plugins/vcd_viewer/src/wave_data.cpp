#include "vcd_viewer/wave_data.h"
#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator/event.h"
#include "netlist_simulator/signal_value.h"
#include <math.h>
#include <vector>


namespace hal {

    WaveData::WaveData(u32 id_, const QString& nam, const QMap<int, int> &other)
        : QMap<int,int>(other), mId(id_), mName(nam)
    {;}

    WaveData::WaveData(const Net* n)
        : mId(n->get_id()),
          mName(QString("%1[%2]")
                .arg(QString::fromStdString(n->get_name()))
                .arg(n->get_id()))
    {;}

    WaveData* WaveData::clockFactory(Net *n, int start, int period, int duration)
    {
        WaveData* retval = new WaveData(n);
        int val = start;
        for (int t=0; t<=duration; t+=period/2)
        {
            retval->insert(t,val);
            val = val ? 0 : 1;
        }
        return retval;
    }

    WaveData* WaveData::simulationResultFactory(Net *n, const NetlistSimulator* sim)
    {
        const std::vector<Event>& evts = sim->get_simulation_events(n);
        if (evts.empty()) return nullptr;
        WaveData* retval = new WaveData(n);
        for (const Event& evt : evts)
        {
            int val = -1;
            switch (evt.new_value)
            {
            case SignalValue::Z:    val = -2; break;
            case SignalValue::X:    val = -1; break;
            case SignalValue::ZERO: val =  0; break;
            case SignalValue::ONE:  val =  1; break;
            }
            retval->insert(evt.time,val);
        }
        // no start value defined?
        auto it = retval->lowerBound(0);
        if (it == retval->end() || it.key() > 0)
            retval->insert(0,-1);
        return retval;
    }

    void WaveData::setStartvalue(int val)
    {
        for (auto it=begin(); it!=end(); ++it)
        {
            *it = val;
            val = val ? 0 : 1;
        }
    }

    bool WaveData::insertToggleTime(int t)
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

    QString WaveData::textValue(const QMap<int,int>::const_iterator& it) const
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

}

#include "vcd_viewer/wave_data.h"
#include "math.h"

namespace hal {

    WaveData::WaveData(const QString& nam, const QMap<int, int> &other)
        : QMap<int,int>(other), mName(nam)
    {;}

    WaveData* WaveData::clockFactory(int start, int cycle, int duration)
    {
        WaveData* retval = new WaveData(QString("clk(%1)").arg(cycle));
        int val = start;
        for (int t=0; t<duration; t+=cycle)
        {
            retval->insert(t,val);
            val = val ? 0 : 1;
        }
        retval->insert(duration,val);
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

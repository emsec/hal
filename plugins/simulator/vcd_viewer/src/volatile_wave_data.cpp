#include "vcd_viewer/volatile_wave_data.h"

namespace hal {

    VolatileWaveData::VolatileWaveData(QObject *parent)
        : QObject(parent)
    {;}

    VolatileWaveData::~VolatileWaveData()
    {
        clear();
    }

    void VolatileWaveData::clear()
    {
        for (auto it = mDataMap.begin(); it != mDataMap.end(); ++it)
            delete it.value();
        mDataMap.clear();
    }

    void VolatileWaveData::addOrReplace(u32 id, WaveData* wd)
    {
        auto it = mDataMap.find(id);
        if (it != mDataMap.end())
        {
            delete it.value();
            mDataMap.erase(it);
        }
        mDataMap.insert(id,wd);
    }

    void VolatileWaveData::ready()
    {
        Q_EMIT triggerRepaint();
    }

    QList<VolatileWaveData::VolatileWaveItem> VolatileWaveData::yPositionData() const
    {
        QList<VolatileWaveItem> retval;
        for (auto itPos = mYposition.constBegin(); itPos!=mYposition.constEnd(); ++itPos)
        {
            auto itWave = mDataMap.constFind(itPos.key());
            if (itWave != mDataMap.constEnd())
                retval.append(VolatileWaveItem{itPos.key(),itPos.value(),itWave.value()});
        }
        return retval;
    }
}

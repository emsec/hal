#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "netlist_simulator_controller/wave_data_provider.h"
#include "hal_core/netlist/net.h"
#include <math.h>
#include <vector>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDir>
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
        : mId(other.mId), mFileIndex(other.mFileIndex), mFileSize(other.mFileSize), mTimeframeSize(other.mTimeframeSize),
          mName(other.mName), mNetType(other.mNetType), mBits(other.mBits), mValueBase(other.mValueBase),
          mData(other.mData), mDirty(true)
    {;}

    WaveData::WaveData(u32 id_, const QString& nam, NetType tp, const QMap<u64,int> &dat)
        : mId(id_), mFileIndex(-1), mFileSize(0), mTimeframeSize(0), mName(nam), mNetType(tp), mBits(1), mValueBase(16), mData(dat), mDirty(true)
    {;}

    WaveData::WaveData(const Net* n, NetType tp)
        : mId(n->get_id()), mFileIndex(-1), mFileSize(0), mTimeframeSize(0),
          mName(QString::fromStdString(n->get_name())),
          mNetType(tp), mBits(1), mValueBase(16), mDirty(true)
    {;}

    void WaveData::resetWave()
    {
        mData.clear();
    }

    void WaveData::setFileSize(u64 siz)
    {
        mFileSize = siz;
        mTimeframeSize = siz;
    }

    WaveData::LoadPolicy WaveData::loadPolicy() const
    {
        u64 maxSizeLoadable = NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable();
        if (mFileSize < maxSizeLoadable) return LoadAllData;
        if (mTimeframeSize && mTimeframeSize < maxSizeLoadable) return LoadTimeframe;
        return TooBigToLoad;
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

    int WaveData::get_value_at(u64 t)
    {
        if (loadPolicy()==LoadAllData)
            loadDataUnlessAlreadyLoaded();
        return intValue(t);
    }

    SaleaeDirectoryNetEntry::Type WaveData::composedType() const
    {
        switch (mNetType)
        {
        case NetGroup: return SaleaeDirectoryNetEntry::Group;
        case BooleanNet: return SaleaeDirectoryNetEntry::Boolean;
        case TriggerTime: return SaleaeDirectoryNetEntry::Trigger;
        default: break;
        }
        return SaleaeDirectoryNetEntry::None;
    }

    bool WaveData::isEqual(const WaveData& other, int tolerance) const
    {
        if (mFileSize != other.mFileSize) return false;
        if (loadPolicy() == LoadAllData)
        {
            if (mData.size() != other.mData.size()) return false;
            auto jt = other.mData.constBegin();
            for (auto it = mData.begin(); it != mData.end(); ++it)
            {
                if (it.value() != jt.value()) return false;
                if (abs((int64_t)it.key()-(int64_t)jt.key()) > tolerance) return false;
                ++jt;
            }
            return true;
        }
        u64 t0 = 0;
        for (;;)
        {
            std::vector<std::pair<u64,int>> evtsThis = get_events(t0);
            std::vector<std::pair<u64,int>> evtsOther = other.get_events(t0);
            if (evtsThis.empty()&&evtsOther.empty()) return true; // all tested
            if (evtsThis.size()!=evtsOther.size()) return false;
            auto jt = evtsOther.begin();
            for (auto it = evtsThis.begin(); it != evtsThis.end(); ++it)
            {
                if (it->second != jt->second) return false;
                if (abs((int64_t)it->first-(int64_t)jt->first)> tolerance) return false;
                ++jt;
                t0 = it->first;
            }
            t0++;
        }
    }

    int WaveData::dataIndex() const
    {
        return mWaveDataList->waveIndexByNetId(mId);
    }

    std::vector<std::pair<u64,int>> WaveData::get_triggered_events(const WaveDataTrigger* wdTrig, u64 t0)
    {
        std::vector<std::pair<u64,int>> retval = wdTrig->get_events(t0);
        for (auto it=retval.begin(); it!=retval.end(); ++it)
        {
            u64 t = it->first;
            it->second = intValue(t);
        }
        return retval;
    }

    std::vector<std::pair<u64,int>> WaveData::get_events(u64 t0) const
    {
        std::vector<std::pair<u64,int>> retval;
        if (loadPolicy() == LoadAllData)
        {       
            for (auto it = mData.lowerBound(t0); it != mData.constEnd(); ++it)
                retval.push_back(std::make_pair(it.key(),it.value()));
        }
        else
        {
            WaveDataProvider* wdp = nullptr;
            std::string saleaeDirectory = mWaveDataList->saleaeDirectory().get_filename();
            switch (mNetType)
            {
            case WaveData::NetGroup:
            {
                const WaveDataGroup* wdGrp = static_cast<const WaveDataGroup*>(this);
                wdp = new WaveDataProviderGroup(saleaeDirectory, wdGrp->children());
                break;
            }
            case WaveData::BooleanNet:
            {
                const WaveDataBoolean* wdBool = static_cast<const WaveDataBoolean*>(this);
                wdp = new WaveDataProviderBoolean(saleaeDirectory, wdBool->children(), wdBool->truthTable());
                break;
            }
            case WaveData::TriggerTime:
            {
                const WaveDataTrigger* wdTrig = static_cast<const WaveDataTrigger*>(this);
                wdp = new WaveDataProviderTrigger(saleaeDirectory, wdTrig->children(), wdTrig->toValueList(), wdTrig->get_filter_wave());
                break;
            }
            default:
                break;
            }

            if (wdp)
            {
                SaleaeDataTuple sdt = wdp->startValue(t0);
                while (sdt.mValue != SaleaeDataTuple::sReadError)
                {
                    retval.push_back(std::make_pair(sdt.mTime,sdt.mValue));
                    sdt = wdp->nextPoint();
                }
                return retval;
            }
            if (mWaveDataList && mFileIndex>=0)
            {
                std::filesystem::path path = mWaveDataList->saleaeDirectory().get_datafile_path(mFileIndex);
                if (!path.empty())
                {
                    SaleaeInputFile sif(path);
                    if (t0)
                    {
                        if (sif.get_file_position(t0,true) < 0)
                            return retval;
                    }
                    SaleaeDataBuffer* sdb = sif.get_buffered_data(NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable());
                    if (sdb)
                    {
                        for (u64 i=0; i<sdb->mCount; i++)
                            retval.push_back(std::make_pair(sdb->mTimeArray[i],sdb->mValueArray[i]));
                        delete sdb;
                    }
                }
            }
        }
        return retval;
    }

    void WaveData::loadDataUnlessAlreadyLoaded()
    {
        if ((u64)mData.size() >= mFileSize) return;
        loadSaleae();
    }

    bool WaveData::loadSaleae(const WaveDataTimeframe& tframe)
    {
        resetWave();
        if (!mWaveDataList || mFileIndex<0) return false;
        std::filesystem::path path = mWaveDataList->saleaeDirectory().get_datafile_path(mFileIndex);
        if (path.empty()) return false;
        SaleaeInputFile sif(path);
        if (!sif.good()) return false;
        u64 t0 = tframe.hasUserTimeframe() ? tframe.sceneMinTime() : 0;
        u64 t1 = tframe.hasUserTimeframe() ? tframe.sceneMaxTime() : 0;
        Q_ASSERT(t0 <= t1);

        int lastVal = BooleanFunction::X;
        bool valuePending = false;
        while (sif.good())
        {
            SaleaeDataBuffer* sdb = sif.get_buffered_data(NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable());
            if (!sdb) break;
            for (u64 i=0; i<sdb->mCount; i++)
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
                else if (!t1 || t <= t1)
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
            delete sdb;
        }
        mDirty = true;
        return true;
    }

    void WaveData::saveSaleae()
    {
        if (!mWaveDataList) return;
        SaleaeDirectory& sd = mWaveDataList->saleaeDirectory();
        SaleaeDirectoryStoreRequest save(&sd);
        std::string nam = mName.toStdString();
        mFileIndex = sd.get_datafile_index(nam,mId);
        if (mFileIndex < 0)
        {
            mFileIndex = sd.get_next_available_index();
            QDir saleaeDir(QString::fromStdString(sd.get_directory()));
            if (!saleaeDir.exists()) saleaeDir.mkpath(saleaeDir.absolutePath());
        }

        SaleaeDirectoryNetEntry sdne(nam,mId);
        sdne.addIndex(SaleaeDirectoryFileIndex(mFileIndex,0,maxTime(),mData.size()));
        sd.add_or_replace_net(sdne);

        mFileSize = mData.size();

        SaleaeDataBuffer sdb(mFileSize);
        int j = 0;
        for (auto it = mData.constBegin(); it != mData.constEnd(); ++it)
        {
            sdb.mTimeArray[j] = it.key();
            sdb.mValueArray[j] = it.value();
            ++j;
        }
        SaleaeOutputFile sof(sd.get_datafile_path(mFileIndex),mFileIndex);
        sof.put_data(&sdb);
    }

    QMap<u64,int>::const_iterator WaveData::timeIterator(double t) const
    {
        if (t<0) return mData.constEnd();
        QMap<u64,int>::const_iterator retval = mData.upperBound((u64)floor(t));
        if (retval != mData.constBegin()) --retval;
        return retval;
    }

    u64 WaveData::neighborTransition(double t, bool next) const
    {
        u64 notFound = (u64) floor(t);
        LoadPolicy lpol = loadPolicy();
        if (lpol ==LoadAllData ||
                (lpol == LoadTimeframe && !mData.isEmpty() && t>=mData.firstKey() && t < mData.lastKey() ))
        {
            if (next)
            {
                QMap<u64,int>::const_iterator it = mData.upperBound((u64)floor(t));
                if (it == mData.end()) return notFound;
                return it.key();
            }
            else
            {
                QMap<u64,int>::const_iterator it = mData.lowerBound((u64)floor(t));
                if (it == mData.begin()) return notFound;
                --it;
                return it.key();
            }
        }

        QList<WaveData*> childList;
        switch (mNetType)
        {
        case NetGroup:
        {
            const WaveDataGroup* wdGrp = static_cast<const WaveDataGroup*>(this);
            childList = wdGrp->children();
            break;
        }
        case BooleanNet:
        {
            const WaveDataBoolean* wdBool = static_cast<const WaveDataBoolean*>(this);
            childList = wdBool->children();
            break;
        }
            // case TriggerNet: overwrite
        default:
            break;
        }

        if (!childList.isEmpty())
        {
            bool first = true;
            u64 retval = notFound;
            for (const WaveData* wd : childList)
            {
                double tChild = wd->neighborTransition(t,next);
                if (tChild == notFound) continue;
                if (first)
                {
                    retval = tChild;
                    first = false;
                }
                else if (next)
                {
                    if (tChild < retval) retval = tChild;
                }
                else
                {
                    if (tChild > retval) retval = tChild;
                }
            }
            return retval;
        }


        if (!mWaveDataList) return notFound;
        SaleaeInputFile sif(mWaveDataList->saleaeDirectory().get_datafile_path(mFileIndex));
        if (!sif.good()) return notFound;

        int64_t pos = sif.get_file_position(t,next);

        while (pos >= 0 && pos <= (int64_t) sif.header()->numTransitions())
        {
            SaleaeDataTuple sdt = sif.get_next_value();
            if (sdt.readError()) return notFound;
            if (next)
            {
                if (sdt.mTime > t) return sdt.mTime;
                ++pos;
            }
            else
            {
                if (sdt.mTime < t) return sdt.mTime;
                --pos;
            }
            sif.set_file_position(pos);
        }

        return notFound;
    }

    int WaveData::intValue(double t) const
    {
        LoadPolicy lpol = loadPolicy();
        if (lpol == LoadAllData ||
                (lpol == LoadTimeframe && !mData.isEmpty() && t>=mData.firstKey() && t < mData.lastKey() ))
        {
            if (mData.isEmpty()) return -1;
            QMap<u64,int>::const_iterator it = timeIterator(t);
            return it.value();
        }

        if (!mWaveDataList) return -1;
        SaleaeInputFile sif(mWaveDataList->saleaeDirectory().get_datafile_path(mFileIndex));
        if (!sif.good()) return -1;
        return sif.get_int_value(t);
    }

    QString WaveData::strValue(double t) const
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
        return stringValue(val,bits(),mValueBase);
    }

    QString WaveData::stringValue(int val, int bits, int base)
    {
        switch (val) {
        case -2 : return "z";
        case -1 : return "x";
        }
        if (bits <= 1 || !val)
            return QString::number(val);
        if (base<0)
        {
            int mask = 1 << (bits-1);
            if (val&mask)
                return QString("-%1").arg((1 << bits) - val);
            else
                return QString::number(val);
        }
        int nDigits = 0;
        switch (base)
        {
        case 2:
            nDigits = bits;
            return QString("0b%1").arg((uint)val,nDigits,2,QLatin1Char('0'));
        case 16:
            nDigits = bits / 4;
            return QString("0x%1").arg((uint)val,nDigits,16,QLatin1Char('0'));
        default: break;
        }
        return QString::number(val,base);
    }

    std::string WaveData::fileName() const
    {
        if (!mWaveDataList || mFileIndex < 0) return std::string();
        return mWaveDataList->saleaeDirectory().get_datafile_path(mFileIndex);
    }

    u64 WaveData::maxTime() const
    {
        if (mData.isEmpty()) return 0;
        return mData.lastKey();
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
    WaveDataBoolean::WaveDataBoolean(WaveDataList* wdList, const QList<WaveData*>& boolInput, const QList<int>& acceptMask)
        : WaveData(wdList->nextBooleanId(),"",WaveData::BooleanNet), mInputCount(boolInput.size()),
          mInputWaves(nullptr), mTruthTable(nullptr)
    {
        mWaveDataList = wdList;
        if (!mInputCount) return;
        if (mInputCount > 16)
        {
            mInputCount = 0;
            return;
        }

        mInputWaves = new WaveData*[mInputCount];
        for (int i = 0; i<mInputCount; i++)
            mInputWaves[i] = boolInput.at(i);
        rename(QString("boolean_%1").arg(id()));


        int truthTableLen = (1 << mInputCount);
        int nByte = (truthTableLen+7) / 8;
        mTruthTable = new char[nByte];
        memset (mTruthTable, 0, nByte);

        for (int accept : acceptMask)
        {
            int j = accept/8;
            int k = accept%8;
            mTruthTable[j] |= (1<<k);
        }
        mWaveDataList->registerBoolean(this);
    }

    WaveDataBoolean::WaveDataBoolean(WaveDataList* wdList, QString boolFunc)
        : WaveData(wdList->nextBooleanId(),boolFunc,WaveData::BooleanNet),
          mInputCount(0), mInputWaves(nullptr), mTruthTable(nullptr)
    {
        mWaveDataList = wdList;
        auto bf = BooleanFunction::from_string(boolFunc.toStdString());
        if (bf.is_error()) return;
        std::vector<std::string> netNames;
        for (std::string netName : bf.get().get_variable_names())
            netNames.push_back(netName);
        if ((mInputCount = netNames.size()) <= 0) return;
        if (mInputCount > 16)
        {
            mInputCount = 0;
            return;
        }
        mInputWaves = new WaveData*[mInputCount];

        bool failed = false;
        for (int i = 0; i<mInputCount; i++)
        {
            const std::string& netName = netNames.at(i);
            WaveData* wd = mWaveDataList->waveDataByName(netName);
            if (!wd)
            {
                failed = true;
                break;
            }
            mInputWaves[i] = wd;
        }

        if (!failed)
        {
            auto tt = bf.get().compute_truth_table(netNames);
            if (tt.is_error())
                failed = true;
            else
            {
                /* dump truth table
                std::cerr << "----" << std::endl;
                for (const std::string& nam : bf.get().get_variable_names())
                {
                    std::cerr << " " << nam;
                }
                std::cerr << std::endl;
                for (std::vector<BooleanFunction::Value> row : tt.get())
                {
                    for (BooleanFunction::Value val : row)
                        std::cerr << " " << BooleanFunction::to_string(val);
                    std::cerr << std::endl;
                }
                std::cerr << "----" << std::endl;
                */

                int truthTableLen = (1 << mInputCount);
                int nByte = (truthTableLen+7) / 8;
                mTruthTable = new char[nByte];
                memset (mTruthTable, 0, nByte);
                for (int i=0; i<truthTableLen; i++)
                {
                    switch (tt.get().at(0).at(i))
                    {
                    case BooleanFunction::Z:
                    case BooleanFunction::X:
                        failed = true;
                        break;
                    case BooleanFunction::ZERO:
                        break;
                    case BooleanFunction::ONE:
                        int j = i/8;
                        int k = i%8;
                        mTruthTable[j] |= (1<<k);
                        break;
                    }
                    if (failed) break;
                }
            }
        }

        if (failed)
        {
            delete [] mInputWaves;
            mInputWaves = 0;
            mInputCount = 0;
            return;
        }
        else
            mWaveDataList->registerBoolean(this);
    }

    WaveDataBoolean::~WaveDataBoolean()
    {
        if (mInputWaves) delete [] mInputWaves;
        if (mTruthTable) delete [] mTruthTable;
    }

    void WaveDataBoolean::recalcData()
    {
        mData.clear();
        switch (loadPolicy())
        {
        case WaveData::TooBigToLoad:
            return;
        case WaveData::LoadTimeframe:
            for (int i=0; i<mInputCount; i++)
                if (mInputWaves[i]->data().isEmpty())
                    return;
            break;
        default:
            break;
        }

        QMap<u64,int> transitionTime;
        // TODO : not loadable
        for (int i=0; i<mInputCount; i++)
        {
            mInputWaves[i]->loadDataUnlessAlreadyLoaded();
            for (u64 t : mInputWaves[i]->data().keys())
                ++transitionTime[t];
        }

        int lastval = SaleaeDataTuple::sReadError;
        for (u64 t : transitionTime.keys())
        {
            int ttInx = 0;
            for (int i=0; i<mInputCount; i++)
            {
                int val = mInputWaves[i]->get_value_at(t);
                if (val < 0 || val > 1)
                {
                    ttInx = 1;
                    break;
                }
                if (val == 1)
                    ttInx |= (1<<i);
            }
            int nextval = SaleaeDataTuple::sReadError;
            if (ttInx < 0)
                nextval = -1;
            else
            {
                int j = ttInx / 8;
                int k = ttInx % 8;
                nextval = (mTruthTable[j] & (1<<k)) ? 1 : 0;
            }
            if (nextval != lastval)
            {
                mData.insert(t,nextval);
                lastval = nextval;
            }
        }
    }

    QList<WaveData*> WaveDataBoolean::children() const
    {
        QList<WaveData*> retval;
        for (int i=0; i<mInputCount; i++)
            retval.append(mInputWaves[i]);
        return retval;
    }

    WaveData::LoadPolicy WaveDataBoolean::loadPolicy() const
    {
        LoadPolicy retval = LoadAllData;
        for (int i=0; i<mInputCount; i++)
        {
            const WaveData* wd = mInputWaves[i];
            switch (wd->loadPolicy())
            {
            case TooBigToLoad:
                return TooBigToLoad;
            case LoadTimeframe:
                retval = LoadTimeframe;
                break;
            default:
                break;
            }
        }
        return retval;
    }

    int WaveDataBoolean::intValue(double t) const
    {
        u32 mask = 1;
        int val = 0;
        for (int i=0; i<mInputCount; i++)
        {
            int childVal = mInputWaves[i]->intValue(t);
            if (childVal < 0) return childVal;
            if (childVal) val |= mask;
            mask <<= 1;
        }
        return (mTruthTable[val/8] & (1<<val%8)) ? 1 : 0;
    }

//--------------------------------------------
    WaveDataTrigger::WaveDataTrigger(WaveDataList* wdList, const QList<WaveData *> &wdTrigger, const QList<int>& toVal)
        : WaveData(wdList->nextTriggerId(),"",TriggerTime),
          mTriggerCount(wdTrigger.size()), mTriggerWaves(nullptr), mFilterWave(nullptr), mToValue(nullptr)
    {
        mWaveDataList = wdList;
        rename(QString("trigger%1").arg(id()));
        if (!mTriggerCount) return;
        mTriggerWaves = new WaveData*[mTriggerCount];
        mToValue = new int[mTriggerCount];
        int nVal = toVal.size();
        for (int i=0; i<mTriggerCount; i++)
        {
            mTriggerWaves[i] = wdTrigger.at(i);
            mToValue[i] = i < nVal ? toVal[i] : -1;
        }
        wdList->registerTrigger(this);
    }

    WaveDataTrigger::~WaveDataTrigger()
    {
        if (mTriggerWaves) delete [] mTriggerWaves;
        if (mToValue) delete [] mToValue;
    }

    void WaveDataTrigger::set_filter_wave(WaveData* wd)
    {
        if (wd && (wd->netType() == WaveData::NetGroup || wd->netType() == WaveData::TriggerTime)) return;
        mFilterWave = wd;
        recalcData();
        SaleaeDirectoryComposedEntry sdce = mWaveDataList->saleaeDirectory().get_composed(id(),SaleaeDirectoryNetEntry::Trigger);
        if (!sdce.isNull())
        {
            SaleaeDirectoryStoreRequest save(&mWaveDataList->saleaeDirectory());
            SaleaeDirectoryNetEntry trigEntry(wd->name().toStdString(),wd->id(),wd->composedType());
            sdce.set_filter_entry(trigEntry.uniqueKey());
            mWaveDataList->saleaeDirectory().add_or_replace_composed(sdce);
        }
    }

    int WaveDataTrigger::intValue(double t) const
    {
        if (loadPolicy() == LoadAllData)
        {
            for (int i=0; i<mTriggerCount; i++)
            {
                mTriggerWaves[i]->loadDataUnlessAlreadyLoaded();
                auto it = mTriggerWaves[i]->data().find(floor(t+0.5));
                if (it != mTriggerWaves[i]->data().constEnd())
                {
                    if (mToValue[i] < 0 || mToValue[i] == it.value()) return 1;
                }
            }
            return 0;
        }
        // TODO : check disk file
        return 0;
    }

    void WaveDataTrigger::recalcData()
    {
        mData.clear();
        switch (loadPolicy())
        {
        case TooBigToLoad:
            return;
        case LoadTimeframe:
            for (int i=0; i<mTriggerCount; i++)
                if (mTriggerWaves[i]->data().isEmpty())
                    return;
            break;
        default:
            for (int i=0; i<mTriggerCount; i++)
                mTriggerWaves[i]->loadDataUnlessAlreadyLoaded();
            break;
        }

        for (int i=0; i<mTriggerCount; i++)
            for (auto it = mTriggerWaves[i]->data().constBegin(); it != mTriggerWaves[i]->data().constEnd(); ++it)
                if (mToValue[i] < 0 || mToValue[i] == it.value())
                    if (!mFilterWave || mFilterWave->intValue(it.key())==1)
                        mData.insert(it.key(),1);
    }

    WaveData::LoadPolicy WaveDataTrigger::loadPolicy() const
    {
        LoadPolicy retval = WaveData::LoadAllData;
        for (int i=0; i<=mTriggerCount; i++)
        {
            const WaveData* wd = nullptr;
            if (i<mTriggerCount)
                wd = mTriggerWaves[i];
            else
                wd = mFilterWave;
            if (!wd) continue;
            switch (wd->loadPolicy())
            {
            case TooBigToLoad:
                return TooBigToLoad;
            case LoadTimeframe:
                retval = LoadTimeframe;
                break;
            default:
                break;
            }
        }
        return retval;
    }

    QList<WaveData*> WaveDataTrigger::children() const
    {
        QList<WaveData*> retval;
        for (int i=0; i<mTriggerCount; i++)
            retval.append(mTriggerWaves[i]);
        return retval;
    }

    u64 WaveDataTrigger::neighborTransition(double t, bool next) const
    {
        if (!mData.empty() && loadPolicy() != TooBigToLoad)
        {
            if (next)
            {
                auto it = mData.upperBound(t);
                if (it != mData.constEnd()) return it.key();
                return t;
            }
            auto it = mData.lowerBound(t);
            if (it == mData.constEnd()) --it;
            while (it != mData.constBegin() && it.key()>=t) --it;
            return it.key()>=t ? t : it.key();
        }
        //TODO : from file
        return t;
    }

    QList<int> WaveDataTrigger::toValueList() const
    {
        QList<int> retval;
        for (int i=0; i<mTriggerCount; i++)
            retval.append(mToValue[i]);
        return retval;
    }

//--------------------------------------------
    WaveDataGroup::WaveDataGroup(WaveDataList *wdList, int grpId, const QString& nam)
        : WaveData(grpId,nam,WaveData::NetGroup)
    {
        mWaveDataList = wdList;
        mWaveDataList->registerGroup(this);
    }

    WaveDataGroup::WaveDataGroup(WaveDataList* wdList, const QString& nam)
        : WaveData(wdList->nextGroupId(),nam,WaveData::NetGroup)
    {
        mWaveDataList = wdList;
        if (nam.isEmpty()) rename(QString("group_%1").arg(id()));
        mWaveDataList->registerGroup(this);
    }

    WaveDataGroup::WaveDataGroup(WaveDataList* wdList, const WaveData* wdGrp)
        : WaveData(wdList->nextGroupId(),wdGrp->name(),WaveData::NetGroup)
    {
        mWaveDataList = wdList;
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

    void WaveDataGroup::add_waveform(WaveData* wd)
    {
        QVector<WaveData*> wds;
        wds.append(wd);
        mWaveDataList->addWavesToGroup(id(),wds);
    }

    void WaveDataGroup::remove_waveform(WaveData* wd)
    {
        int irow = mIndex.value(WaveDataGroupIndex(wd),-1);
        if (irow < 0) return;
        removeAt(irow);
        restoreIndex();
        recalcData();
        int iwave = mWaveDataList->waveIndexByNetId(wd->id());
        if (iwave >= 0) mWaveDataList->emitWaveRemovedFromGroup(iwave,id());
    }

    std::vector<WaveData*> WaveDataGroup::get_waveforms() const
    {
        std::vector<WaveData*> retval;
        for (WaveData* wd : mGroupList) retval.push_back(wd);
        return retval;
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
        SaleaeDirectoryComposedEntry sdce(get_name(),id(),SaleaeDirectoryNetEntry::Group);
        mIndex.clear();
        int inx = 0;
        for (const WaveData* wd : mGroupList)
        {
            sdce.add_child(wd->id());
            mIndex[WaveDataGroupIndex(wd)] = inx++;
        }
        mWaveDataList->saleaeDirectory().add_or_replace_composed(sdce);
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

    WaveData::LoadPolicy WaveDataGroup::loadPolicy() const
    {
        WaveData::LoadPolicy retval = WaveData::LoadAllData;
        for (const WaveData* wd : mGroupList)
        {
            switch (wd->loadPolicy())
            {
            case WaveData::TooBigToLoad:
                return WaveData::TooBigToLoad;
            case WaveData::LoadTimeframe:
                retval = WaveData::LoadTimeframe;
                break;
            default:
                break;
            }
        }
        return retval;
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
            if (wd->loadPolicy()==WaveData::TooBigToLoad)
            {
                // Would block, must determine group values in background thread
                return;
            }
            else if (wd->data().size() < (int) wd->fileSize())
            {
                // Loadable but nut loaded yet
                wd->loadSaleae(mWaveDataList->timeFrame());
            }
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
            int mask = (1 << (nChildren - ibit - 1));
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

    int WaveDataGroup::intValue(double t) const
    {
        u32 mask = 1 << (mGroupList.size()-1);
        int retval = 0;
        for (const WaveData* wd : mGroupList)
        {
            int childVal = wd->intValue(t);
            if (childVal < 0) return childVal;
            if (childVal) retval |= mask;
            mask >>= 1;
        }
        return retval;
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
          mSaleaeDirectory(sdFilename.toStdString()), mMaxGroupId(0), mMaxBooleanId(0), mMaxTriggerid(0)
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

    void WaveDataList::emitTimeframeChanged()
    {
        Q_EMIT timeframeChanged(&mTimeframe);
    }

    void WaveDataList::incrementSimulTime(u64 deltaT)
    {
        mTimeframe.mSimulateMaxTime += deltaT;
        if (mTimeframe.mSimulateMaxTime > mTimeframe.mSceneMaxTime)
            setMaxTime(mTimeframe.mSimulateMaxTime);
    }

    void WaveDataList::setUserTimeframe(u64 t0, u64 t1)
    {
        if (t0 == mTimeframe.mUserdefMinTime && t1 == mTimeframe.mUserdefMaxTime) return;
        mTimeframe.setUserTimeframe(t0,t1);
        for (auto it=begin(); it!=end(); ++it)
        {
            WaveData* wd = *it;
            // invalidate memory data
            if (wd->loadPolicy()==WaveData::LoadTimeframe)
            {

                wd->setTimeframeSize(0);
                wd->clear();
            }
        }
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
            mTrashCan.append(*it);
        clear();
        emptyTrash();
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

    void WaveDataList::updateWaveName(int iwave, const QString& nam)
    {
        if (at(iwave)->rename(nam))
        {
            SaleaeDirectoryStoreRequest save(&mSaleaeDirectory);
            Q_EMIT waveRenamed(iwave);
            mSaleaeDirectory.rename_net(at(iwave)->id(),nam.toStdString());
        }
    }

    void WaveDataList::updateGroupName(u32 grpId, const QString &nam)
    {
        WaveDataGroup* grp = mDataGroups.value(grpId);
        if (grp && grp->rename(nam))
        {
            SaleaeDirectoryStoreRequest save(&mSaleaeDirectory);
            Q_EMIT groupRenamed(grpId);
        }
    }

    void WaveDataList::emitWaveRemovedFromGroup(int iwave, int grpId)
    {
        Q_EMIT waveRemovedFromGroup(iwave, grpId);
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

    void WaveDataList::emptyTrash()
    {
        auto it = mTrashCan.begin();
        while (it != mTrashCan.end())
        {
            if ((*it)->hasSubscriber())
                ++it;
            else
            {
                delete (*it);
                it = mTrashCan.erase(it);
            }
        }
    }

    void WaveDataList::updateWaveData(int inx)
    {
        WaveData* wd = at(inx);
        wd->saveSaleae();
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
        if (tmax>mTimeframe.mSceneMaxTime)
            setMaxTime(tmax);
    }

    void WaveDataList::add(WaveData* wd, bool updateSaleae)
    {
        int n = size();

        mIds[wd->id()] = n;
        append(wd);
        wd->setWaveDataList(this);
        updateMaxTime();
        if (updateSaleae) wd->saveSaleae();
//        if (!silent) Q_EMIT waveAdded(n);
        testDoubleCount();
    }

    void WaveDataList::triggerAddToView(u32 id) const
    {
        int iwave = waveIndexByNetId(id);
        if (iwave<0) return;
        // ugly but save. While emit is not const because of Q_SIGNAL syntax it will not modify object
        const_cast<WaveDataList*>(this)->emitWaveAdded(iwave);
    }

    void WaveDataList::registerTrigger(WaveDataTrigger *wdTrig)
    {
        u32 trigId = wdTrig->id();
        Q_ASSERT(!mDataTrigger.contains(trigId));
        mDataTrigger.insert(trigId,wdTrig);
        SaleaeDirectoryComposedEntry sdce(wdTrig->name().toStdString(),trigId,SaleaeDirectoryNetEntry::Trigger);
        for (WaveData* wd : wdTrig->children())
        {
            sdce.add_child(wd->id());
        }
        std::vector<int> toValue;
        for (int tval : wdTrig->toValueList())
            toValue.push_back(tval);
        sdce.set_data(toValue);

        WaveData* wdFilt = wdTrig->get_filter_wave();
        if (wdFilt)
        {
            SaleaeDirectoryNetEntry filterEntry(wdFilt->name().toStdString(),wdFilt->id(),wdFilt->composedType());
            sdce.set_filter_entry(filterEntry.uniqueKey());
        }
        mSaleaeDirectory.add_or_replace_composed(sdce);
        Q_EMIT triggerAdded(trigId);
    }

    void WaveDataList::registerBoolean(WaveDataBoolean *wdBool)
    {
       u32 boolId = wdBool->id();
       Q_ASSERT(!mDataBooleans.contains(boolId));
       mDataBooleans.insert(boolId,wdBool);
       SaleaeDirectoryComposedEntry sdce(wdBool->name().toStdString(),boolId,SaleaeDirectoryNetEntry::Boolean);
       int n = 1;
       for (WaveData* wd : wdBool->children())
       {
           n <<= 1;
           sdce.add_child(wd->id());
       }
       std::vector<int> acceptVal;
       const char* ttable = wdBool->truthTable();
       for (int i=0; i<n; i++)
       {
           int j = i/8;
           int k = i%8;
           if (ttable[j] & (1<<k)) acceptVal.push_back(i);
       }
       sdce.set_data(acceptVal);
       mSaleaeDirectory.add_or_replace_composed(sdce);
       Q_EMIT booleanAdded(boolId);
    }

    void WaveDataList::registerGroup(WaveDataGroup *grp)
    {
        u32 grpId = grp->id();
        if (!grpId) return;
        Q_ASSERT(!mDataGroups.contains(grpId));
        mDataGroups.insert(grpId,grp);
        if (grpId)
        {
            mSaleaeDirectory.add_or_replace_composed(SaleaeDirectoryComposedEntry(grp->name().toStdString(),grpId,SaleaeDirectoryNetEntry::Group));
            updateMaxTime();
            Q_EMIT groupAdded(grpId);
        }
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
        wd->saveSaleae();
    }

    void WaveDataList::removeGroup(u32 grpId)
    {
        WaveDataGroup* grp = mDataGroups.value(grpId);
        if (!grp) return;
        Q_EMIT groupAboutToBeRemoved(grp);
        mSaleaeDirectory.remove_composed(grpId,SaleaeDirectoryNetEntry::Group);
        mTrashCan.append(grp);
    }

    void WaveDataList::replaceWaveData(int inx, WaveData* wdNew)
    {
        // replace existing
        triggerBeginResetModel();
        WaveData* wdOld = at(inx);
        Q_ASSERT(wdOld);
        Q_ASSERT(wdOld != wdNew);
        wdNew->rename(wdOld->name());
        operator[](inx) = wdNew;
        wdNew->setWaveDataList(this);
        for (WaveDataGroup* grp : mDataGroups.values())
            if (grp->hasNetId(wdNew->id()))
                grp->replaceChild(wdNew);
        if (wdNew->maxTime() > mTimeframe.mSceneMaxTime)
            updateMaxTime();

        wdNew->saveSaleae();


        emitWaveUpdated(inx);
        mTrashCan.append(wdOld);
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
            if (wd->loadPolicy() == WaveData::LoadAllData)
                wd->loadSaleae(mTimeframe);
            emitWaveUpdated(i);
            mTrashCan.append(it.value());
            saleaeWaves.erase(it);
        }

        for (auto it=saleaeWaves.begin(); it!=saleaeWaves.end(); ++it)
        {
            add(it.value(),false);
        }
        setMaxTime(mSaleaeDirectory.get_max_time());

        for (auto it = mDataGroups.begin(); it != mDataGroups.end(); ++it)
        {
            (*it)->recalcData();
        }
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
        if (inx >= 0)
        {
            WaveData* wd = at(inx);
            if (wd->loadPolicy()==WaveData::LoadAllData)
                wd->loadDataUnlessAlreadyLoaded();
            return wd;
        }
        WaveData* wd = new WaveData(n);
        if (wd->loadSaleae(mTimeframe))
        {
            add(wd,false);
            return wd;
        }
        delete wd;
        return nullptr;
    }

    WaveData* WaveDataList::waveDataByName(const std::string& nam) const
    {
        QString needle = QString::fromStdString(nam);
        for (WaveData* wd : *this)
            if (wd->name()==needle)
                return wd;
        for (WaveDataBoolean* wdb : mDataBooleans.values())
            if (wdb->name()==needle)
                return wdb;
        return nullptr;
    }

    WaveData* WaveDataList::waveDataById(const int id)
    {
        for (WaveData* wd : *this)
            if (wd->id() == id)
                return wd;
        for (WaveDataBoolean* wdb : mDataBooleans.values())
            if (wdb->id() == id)
                return wdb;
        return nullptr;
    }

    void WaveDataList::testDoubleCount()
    {
        QMap<u32,int> doubleCount;
        QSet<QString> notInNetlist;
        for (const WaveData* wd : *this)
        {
            if (!wd->id())
            {
                if (!mNotInNetlist.contains(wd->name()))
                    notInNetlist.insert(wd->name());
            }
            else
                ++doubleCount[wd->id()];
        }
        for (auto it=doubleCount.constBegin(); it!=doubleCount.constEnd(); ++it)
        {
            if (it.value() > 1)
            {
                log_warning("simulation_plugin", "Duplicate waveform ({}x) found : '{}'", it.value(), at(mIds.value(it.key()))->name().toStdString());
            }
        }
        if (!notInNetlist.isEmpty())
        {
            for (const QString& name : notInNetlist)
            {
                log_warning("simulation_plugin", "Waveform not in (partial) netlist : '{}'", name.toStdString());
            }
            mNotInNetlist += notInNetlist;
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
        mTrashCan.append(toDelete);
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

#pragma once

#include <QLine>
#include <QHash>
#include <QMutex>
#include <QTextStream>
#include "netlist_simulator_controller/wave_data.h"
#include "waveform_viewer/wave_form_painted.h"

class QScrollBar;
class QThread;

namespace hal {

    class WaveItemHash;

    class WaveItemIndex
    {
    public:
        enum IndexType { Invalid, Wire, Bool, Group };
    private:
        IndexType mType;
        u32 mIndex;
        u32 mParentId;
    public:
        WaveItemIndex(u32 inx = 0, IndexType tp = Invalid, u32 pid = 0)
            : mType(tp), mIndex(inx), mParentId(pid) {;}
        bool isValid() const { return mType != Invalid; }
        bool isGroup() const { return mType == Group; }
        bool isWire() const  { return mType == Wire; }
        u32 intType() const { return mType; }
        u32 index() const { return mIndex; }
        u32 parentId() const { return mParentId; }
        bool operator== (const WaveItemIndex& other) const;
    };

    uint qHash(const WaveItemIndex &wii);

    class WaveItem : public QObject
    {
        Q_OBJECT
    public:
        enum Request { AddRequest, DataChanged, SetVisible, SetPosition, SelectionChanged, DeleteRequest, DeleteAcknowledged };
        enum State { Null, Loading, Aborted, Finished, Painted, Failed };
    private:
        WaveData* mData;
        QThread* mLoader;
        int mLoadProgress;
        State mState;

    public:
        WaveFormPainted mPainted;
        WaveZoomShift mLoadValidity;
        QMutex mMutex;
        bool mVisibleRange;
        bool mLoop;

        enum ColorIndex { Solid, HiLight, Dotted, Background };

    private:
        int   mYposition;
        int   mRequest;
        float mMinTime;
        float mMaxTime;
        float mMaxTransition;
        bool mVisibile;
        bool mSelected;

    public Q_SLOTS:
        void handleWaveLoaderFinished();

    Q_SIGNALS:
        void doneLoading();
        void gotCursorValue();

    public:
        WaveItem(WaveData* dat, QObject* parent = nullptr);
        ~WaveItem();

        bool isNull() const { return mState==Null; }
        bool isLoading() const { return mState==Loading; }
        bool isPainted() const { return mState==Painted; }
        bool isFinished() const { return mState==Finished; }
        bool isAborted() const { return mState==Aborted; }
        bool isSelected() const { return mSelected; }
        bool isThreadBusy() const { return isFinished() || isAborted(); }
        void deletePainted();
        void startGeneratePainted(const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar, const WaveDataTimeframe& tframe);
        void startLoader(const QString& workdir, const WaveTransform* trans, const WaveScrollbar* sbar, const WaveDataTimeframe& tframe);
        void abortLoader();
        void setState(State stat);
        void dump(QTextStream& xout) const;
        bool hasLoader() const { return mLoader != nullptr; }
        void loadSaleae();
        State state() const { return mState; }

   //     int waveIndex() const { return mWaveIndex; }
        const WaveData* wavedata() const { return mData; }
        int yPosition() const { return mYposition; }
        bool isVisibile() const { return mVisibile; }
        void setYposition(int pos);
        void setWaveData(WaveData* wd);
        void setWaveVisible(bool vis);
        void setWaveSelected(bool sel);
        bool setTimeframe();
        bool hasRequest(Request rq) const;
        void setRequest(Request rq);
        void clearRequest(Request rq);
        bool isDeleted() const;
        int loadeProgress() const { return mLoadProgress; }
        void incrementLoadProgress();
        static const char* sBackgroundColor;
        bool isGroup() const { return mData->netType() == WaveData::NetGroup; }
        int cursorValue(double tCursor, int xpos);
    };

    class WaveItemHash : public QHash<WaveItemIndex,WaveItem*>
    {
        int mVisibleEntries;
        QList<WaveItem*> mTrashCan;
    public:
        int visibleEntries() const { return mVisibleEntries; }
        int importedWires() const;
        void setVisibleEntries(int ve) { mVisibleEntries = ve; }
        WaveItem* addOrReplace(WaveData*wd, WaveItemIndex::IndexType tp, int iwave, int parentId);
        void dump(const char* stub);
        void dispose(WaveItem* wi);
        void emptyTrash();
    };
}

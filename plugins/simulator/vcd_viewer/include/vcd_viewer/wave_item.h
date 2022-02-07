#pragma once

#include <QGraphicsItem>
#include <QLine>
#include <QHash>
#include "netlist_simulator_controller/wave_data.h"

class QScrollBar;

namespace hal {

    class WaveItemIndex
    {
    public:
        enum IndexType { Invalid, Wire, Group };
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
        u32 index() const { return mIndex; }
        u32 parentId() const { return mParentId; }
        bool operator== (const WaveItemIndex& other) const;
    };

    uint qHash(const WaveItemIndex &wii);

    class WaveValueAsTextItem : public QGraphicsItem
    {
        int mValue;
        float mWidth;
    public:
        WaveValueAsTextItem(int val, float w, QGraphicsItem* parentItem = nullptr);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        QRectF boundingRect() const override;
    };

    class WaveItem : public QGraphicsItem
    {
    public:
        enum Request { AddRequest, DataChanged, SetVisible, SetPosition, SelectionChanged, DeleteRequest, DeleteAcknowledged };
    private:
        WaveData* mData;
        int       mYposition;
        int       mRequest;
        QVector<QLine> mSolidLines;
        QVector<QLineF> mDotLines;
        QVector<QRectF> mGrpRects;

        float mMinTime;
        float mMaxTime;
        float mMaxTransition;
        bool mVisibile;
        bool mSelected;

        void construct();
        void constructWire();
        void constructGroup();

        void enforceYposition();
        static const char* sLineColor[];
        enum ColorIndex { Solid, HiLight, Dotted, Background };
    public:
        WaveItem(WaveData* dat);
        ~WaveItem();

   //     int waveIndex() const { return mWaveIndex; }
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        const WaveData* wavedata() const { return mData; }
        int yPosition() const { return mYposition; }
        bool waveVisibile() const { return mVisibile; }
        void setYposition(int pos);
        void setWaveData(WaveData* wd);
        void setWaveVisible(bool vis);
        void setWaveSelected(bool sel);
        QRectF boundingRect() const override;
        bool setTimeframe();
        bool hasRequest(Request rq) const;
        void setRequest(Request rq);
        void clearRequest(Request rq);
        bool isDeleted() const;
        static bool sValuesAsText;
    };

    class WaveItemHash : public QHash<WaveItemIndex,WaveItem*>
    {
        int mVisibleEntries;
    public:
        int visibleEntries() const { return mVisibleEntries; }
        int importedWires() const;
        void setVisibleEntries(int ve) { mVisibleEntries = ve; }
        void addOrReplace(WaveData*wd, WaveItemIndex::IndexType tp, int inx, int parentId);
        void dump(const char* stub);
    };
}

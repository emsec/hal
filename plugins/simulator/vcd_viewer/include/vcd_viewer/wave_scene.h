#pragma once

#include <QGraphicsScene>
#include <QHash>
#include <QTimer>
#include "hal_core/defines.h"

namespace hal {
    class WaveItem;
    class WaveDataList;
    class WaveCursor;

    class WaveScene : public QGraphicsScene
    {
        Q_OBJECT

        WaveDataList* mWaveDataList;

        QHash<int,WaveItem*> mWaveItems;
        QHash<int,WaveItem*> mGroupItems;
        WaveCursor* mCursor;
        QTimer* mClearTimer;
        float   mXmag;

        static float yPosition(int irow);
        void addWaveInternal(int iwave, int ypos);
        void addGroupInternal(int grpId, int ypos);
        float adjustSceneRect(u64 tmax = 0);
        int nextWavePosition() const;

        static const int sMinItemHeight;
    Q_SIGNALS:
        void cursorMoved(float xpos);

    public Q_SLOTS:
        void xScaleChanged(float m11);
        void updateWaveItemValues();
        void handleWaveAdded(int iwave);
        void handleWaveUpdated(int iwave);
        void handleGroupAdded(int grpId);
        void setWavePositions(const QHash<int,int>& wpos, const QHash<int, int>& gpos);
        void handleIndexRemoved(int iwave, bool isGroup);
        void handleMaxTimeChanged(u64 tmax);
//        void handleIndexInserted(int iwave, bool isGroup);

    public:
        WaveScene(WaveDataList* wdlist, QObject* parent = nullptr);
        void emitCursorMoved(float xpos);
        void setCursorPosition(const QPointF& pos);
        float xScaleFactor() const { return mXmag; }
        float cursorXposition() const;

        static const float sMinSceneWidth;
    };
}

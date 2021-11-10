#pragma once

#include <QGraphicsScene>
#include <QHash>
#include <QTimer>
#include "hal_core/defines.h"

namespace hal {
    class WaveItem;
    class WaveDataList;
    class VolatileWaveData;
    class WaveCursor;

    class WaveScene : public QGraphicsScene
    {
        Q_OBJECT

        WaveDataList* mWaveDataList;
        VolatileWaveData* mVolatileWaveData;

        QHash<int,WaveItem*> mWaveItems;
        QHash<int,WaveItem*> mVolatileItems;
        QHash<int,int>   mWavePositions;
        int mMaxPosition;
        WaveCursor* mCursor;
        QTimer* mClearTimer;

        static float yPosition(int irow);
        void addWaveInternal(int iwave, int ypos);
        float adjustSceneRect(u64 tmax = 0);

        static const int sMinItemHeight;

    Q_SIGNALS:
        void cursorMoved(float xpos);

    public Q_SLOTS:
        void xScaleChanged(float m11);
        void handleWaveAdded(int iwave);
        void setWavePositions(const QHash<int,int>& wpos);
        void handleVolatileRepaint();
        void handleIndexRemoved(int iwave, bool isVolatile);
//        void handleIndexInserted(int iwave, bool isVolatile);

    public:
        WaveScene(WaveDataList* wdlist, VolatileWaveData* wdVol, QObject* parent = nullptr);
        void emitCursorMoved(float xpos);
        void setCursorPosition(const QPointF& pos);
        float cursorXpostion() const;

        static const float sMinSceneWidth;
    };
}

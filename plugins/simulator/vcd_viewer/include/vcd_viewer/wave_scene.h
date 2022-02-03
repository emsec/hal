#pragma once

#include <QGraphicsScene>
#include <QHash>
#include <QTimer>
#include "hal_core/defines.h"
#include "vcd_viewer/wave_item.h"

namespace hal {
    class WaveItem;
    class WaveDataList;
    class WaveDataTimeframe;
    class WaveCursor;

    class WaveScene : public QGraphicsScene
    {
        Q_OBJECT

        WaveDataList* mWaveDataList;
        WaveItemHash* mWaveItemHash;

        WaveCursor* mCursor;
        QTimer* mClearTimer;
        float   mXmag;
//        QGraphicsRectItem* mSceneRect;

        float adjustSceneRect(const WaveDataTimeframe* tframe);
        int nextWavePosition() const;

        static const int sMinItemHeight;
    Q_SIGNALS:
        void cursorMoved(float xpos);

    public Q_SLOTS:
        void xScaleChanged(float m11);
        void updateWaveItemValues();
        void handleWaveUpdated(int iwave, int groupId);
        void handleTimeframeChanged(const WaveDataTimeframe* tframe);
        void updateWaveItems();

    public:
        WaveScene(WaveDataList* wdlist, WaveItemHash* wHash, QObject* parent = nullptr);
        void emitCursorMoved(float xpos);
        void setCursorPosition(const QPointF& pos);
        float xScaleFactor() const { return mXmag; }
        float cursorXposition() const;

        static const float sMinSceneWidth;
        static float yPosition(int irow);
    };
}

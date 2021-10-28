#pragma once

#include <QGraphicsScene>
#include <QList>
#include <QTimer>

#include "hal_core/defines.h"

class QGraphicsRectItem;

namespace hal {

    class WaveData;
    class WaveItem;
    class WaveCursor;
    class WaveTimescale;
    class WaveIndex;

    class WaveScene : public QGraphicsScene
    {
        Q_OBJECT

        const WaveIndex* mWaveIndex;
        QList<WaveItem*> mWaveItems;
        WaveTimescale* mTimescale;
        WaveCursor* mCursor;
        QTimer* mClearTimer;
        QGraphicsRectItem* mDebugSceneRect;

        float adjustSceneRect(u64 tmax = 0);

        static const int sMinItemHeight;
    Q_SIGNALS:
        void cursorMoved(float xpos);

    public Q_SLOTS:
        void xScaleChanged(float m11);
        void handleWaveAppended(WaveData* wd);
        void handleWaveDataChanged(int inx);
        void handleWaveRemoved(int inx);
        void handleMaxTimeChanged(u64 tmax);

    public:
        WaveScene(const WaveIndex* winx, QObject* parent = nullptr);
        ~WaveScene();
        void emitCursorMoved(float xpos);
        float cursorPos() const;
        void setCursorPos(float xp, bool relative);

        float yPosition(int dataIndex) const;
        void moveToIndex(int indexFrom, int indexTo);
        void deleteWave(int dataIndex);
        void generateDataSample();

        static const float sMinSceneWidth;
    };

}

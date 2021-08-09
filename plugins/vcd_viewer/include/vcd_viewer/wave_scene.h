#pragma once

#include <QGraphicsScene>
#include <QList>
#include <QTimer>

namespace hal {

    class WaveData;
    class WaveItem;
    class WaveCursor;
    class WaveTimescale;

    class WaveScene : public QGraphicsScene
    {
        Q_OBJECT

        QList<WaveItem*> mWaveItems;
        WaveTimescale* mTimescale;
        WaveCursor* mCursor;
        QTimer* mClearTimer;

        float adjustSceneRect();
    Q_SIGNALS:
        void cursorMoved(float xpos);

    public Q_SLOTS:
        void xScaleChanged(float m11);

    public:
        WaveScene(QObject* parent = nullptr);
        ~WaveScene();
        void emitCursorMoved(float xpos);
        float cursorPos() const;
        void setCursorPos(float xp, bool relative);

        int numberWaves() const { return mWaveItems.size(); }
        float yPosition(int dataIndex) const;
        const WaveData* waveData(int dataIndex) const;
        void setWaveData(int dataIndex, WaveData* wd);
        void moveToIndex(int indexFrom, int indexTo);
        void deleteWave(int dataIndex);
        int addWave(WaveData* wd);
        void generateDataSample();
    };

}

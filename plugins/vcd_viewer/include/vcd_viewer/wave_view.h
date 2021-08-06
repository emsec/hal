#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>

namespace hal {

    class WaveLabel;
    class WaveData;

    class WaveView : public QGraphicsView
    {
        Q_OBJECT

        QMap<QString,int> mWaveIndices;

        QVector<WaveLabel*> mValues;
        float mXmag, mYmag, mDx, mDy, mXmagMin;

        float mLastCursorPos;
        int mLastWidth;
        int mCursorPixelPos;

        void restoreCursor();
        void updateLabel(int dataIndex, float xpos);
    Q_SIGNALS:
        void changedXscale(float m11);

    private Q_SLOTS:
        void handleCursorMoved(float xpos);
        void editWaveData(int dataIndex);
        void deleteWave(int dataIndex);

    protected:
        void wheelEvent(QWheelEvent *event) override;
        void scrollContentsBy(int dx, int dy) override;
        void resizeEvent(QResizeEvent *event) override;

    public:
        WaveView(QWidget* parent=nullptr);
        void addOrReplaceWave(WaveData* wd);
        const WaveData* waveDataByName(const QString& name) const;

    };

}

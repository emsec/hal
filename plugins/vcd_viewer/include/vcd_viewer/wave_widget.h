#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>
#include <QSplitter>
#include <QFrame>

namespace hal {

    class WaveLabel;
    class WaveData;
    class WaveView;

    class WaveWidget : public QSplitter
    {
        Q_OBJECT

        /*
        QMap<QString,int> mWaveIndices;

        QVector<WaveLabel*> mValues;

        float mLastCursorPos;
        int mLastWidth;

        void restoreCursor();
        void updateLabel(int dataIndex, float xpos);
    Q_SIGNALS:
        void changedXscale(float m11);

    private Q_SLOTS:
        void handleCursorMoved(float xpos);
        void editWaveData(int dataIndex);
        void deleteWave(int dataIndex);
*/

        WaveView *mWaveView;
        QFrame *mFrame;
    public:
        WaveWidget(QWidget* parent=nullptr);
        void addOrReplaceWave(WaveData* wd);
        const WaveData* waveDataByName(const QString& name) const;
    };

}

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
    class WaveScene;

    class WaveWidget : public QSplitter
    {
        Q_OBJECT

    public:
        WaveWidget(QWidget* parent=nullptr);
        void addOrReplaceWave(WaveData* wd);
        const WaveData* waveDataByName(const QString& name) const;

    private Q_SLOTS:
        void handleCursorMoved(float xpos);
        void handleYScroll(int dy);
        void handleLabelSwap(int isource, int ypos);
        void handleLabelMove(int isource, int ypos);

        void editWaveData(int dataIndex);
        void deleteWave(int dataIndex);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        QMap<QString,int> mWaveIndices;
        QVector<WaveLabel*> mValues;

        void updateLabel(int dataIndex, float xpos);
        void updateIndices();
        int  targetIndex(int ypos);

        WaveView *mWaveView;
        WaveScene *mWaveScene;
        QFrame *mFrame;
    };

}

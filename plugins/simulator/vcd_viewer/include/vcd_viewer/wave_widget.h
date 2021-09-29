#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>
#include <QSplitter>
#include <QFrame>
#include <hal_core/defines.h>

namespace hal {

    class WaveLabel;
    class WaveData;
    class WaveView;
    class WaveScene;
    class SelectionTreeItem;

    class WaveWidget : public QSplitter
    {
        Q_OBJECT

    public:
        WaveWidget(QWidget* parent=nullptr);
        void addOrReplaceWave(WaveData* wd);
        const WaveData* waveDataByNetId(u32 id) const;
        bool isVisulizeNetState() const { return mVisualizeNetState; }

    public Q_SLOTS:
        void setVisualizeNetState(bool state);

    private Q_SLOTS:
        void handleCursorMoved(float xpos);
        void handleYScroll(int dy);
        void handleLabelSwap(int isource, int ypos);
        void handleLabelMove(int isource, int ypos);

        void editWaveData(int dataIndex);
        void deleteWave(int dataIndex);
        void handleSelectionHighlight(const QVector<const SelectionTreeItem*>& highlight);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        QMap<u32,int> mWaveIndices;
        QVector<WaveLabel*> mValues;

        void updateLabel(int dataIndex, float xpos);
        void updateIndices();
        int  targetIndex(int ypos);
        void visualizeCurrentNetState(float xpos);

        WaveView *mWaveView;
        WaveScene *mWaveScene;
        QFrame *mFrame;
        bool mVisualizeNetState;
        u32 mGroupIds[3];
    };

}

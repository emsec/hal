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
    class NetlistSimulatorController;

    class WaveWidget : public QSplitter
    {
        Q_OBJECT

    public:
        WaveWidget(NetlistSimulatorController* ctrl, QWidget* parent=nullptr);
        void addOrReplaceWave(WaveData* wd);
        const WaveData* waveDataByNetId(u32 id) const;
        bool isVisulizeNetState() const { return mVisualizeNetState; }
        u32 controllerId() const;
        NetlistSimulatorController* controller() const { return mController; }
        void setVisualizeNetState(bool state, bool activeTab);

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
        NetlistSimulatorController* mController;
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

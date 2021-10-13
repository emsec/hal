#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>
#include <QSplitter>
#include <QFrame>
#include <hal_core/defines.h>
#include "netlist_simulator_controller/netlist_simulator_controller.h"

namespace hal {

    class WaveLabel;
    class WaveData;
    class WaveView;
    class WaveScene;
    class SelectionTreeItem;

//    enum SimulationState { SimulationSelectGates, SimulationClockSet, SimulationInputGenerate, SimulationShowResults };

    class WaveWidget : public QSplitter
    {
        Q_OBJECT

    public:
        WaveWidget(NetlistSimulatorController* ctrl, QWidget* parent=nullptr);
        ~WaveWidget();
        void addOrReplaceWave(WaveData* wd);
        const WaveData* waveDataByNetId(u32 id) const;
        bool isVisulizeNetState() const { return mVisualizeNetState; }
        u32 controllerId() const;
        NetlistSimulatorController* controller() const { return mController; }
        void setVisualizeNetState(bool state, bool activeTab);
        void takeOwnership(std::unique_ptr<NetlistSimulatorController>& ctrl);
        bool triggerClose();

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
        std::unique_ptr<NetlistSimulatorController> mControllerOwner;
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

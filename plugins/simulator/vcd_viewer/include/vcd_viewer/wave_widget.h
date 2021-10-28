#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>
#include <QSplitter>
#include <QFrame>
#include <hal_core/defines.h>
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "vcd_viewer/wave_index.h"

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
        bool isVisulizeNetState() const { return mVisualizeNetState; }
        u32 controllerId() const;
        NetlistSimulatorController* controller() const { return mController; }
        void setVisualizeNetState(bool state, bool activeTab);
        void takeOwnership(std::unique_ptr<NetlistSimulatorController>& ctrl);
        bool triggerClose();
        void setGates(const std::vector<Gate*>& gats);
        void addResults();
        NetlistSimulatorController::SimulationState state() const;
        void createEngine(const QString& engineFactoryName);

    public Q_SLOTS:
        void handleEngineFinished(bool success);

    private Q_SLOTS:
        void handleCursorMoved(float xpos);
        void handleYScroll(int dy);
        void handleLabelSwap(int isource, int ypos);
        void handleLabelMove(int isource, int ypos);

        void editWaveData(int dataIndex);
        void deleteWave(int dataIndex);
        void handleSelectionHighlight(const QVector<const SelectionTreeItem*>& highlight);
        void handleWaveAppended(WaveData* wd);
        void handleWaveDataChanged(int inx);
        void handleWaveRemoved(int inx);
        void handleStateChanged(NetlistSimulatorController::SimulationState state);

    Q_SIGNALS:
        void stateChanged(NetlistSimulatorController::SimulationState state);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        NetlistSimulatorController* mController;
        std::unique_ptr<NetlistSimulatorController> mControllerOwner;
        WaveIndex mWaveIndex;
        QVector<WaveLabel*> mValues;

        void updateLabel(int dataIndex, float xpos);
        int  targetIndex(int ypos);
        void visualizeCurrentNetState(float xpos);

        WaveView *mWaveView;
        WaveScene *mWaveScene;
        QFrame *mFrame;
        bool mVisualizeNetState;
        u32 mGroupIds[3];
    };

}

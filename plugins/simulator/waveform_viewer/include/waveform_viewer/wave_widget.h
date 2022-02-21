#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>
#include <QSplitter>
#include <QFrame>
#include <hal_core/defines.h>
#include "waveform_viewer/wave_item.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"

namespace hal {

    class WaveGraphicsCanvas;
    class WaveTreeModel;
    class WaveTreeView;
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
        void refreshNetNames();
        void scrollToYpos(int ypos);
        void removeGroup(u32 grpId);
        bool canImportWires() const;

    public Q_SLOTS:
        void handleEngineFinished(bool success);

    private Q_SLOTS:

        void handleSelectionHighlight(const QVector<const SelectionTreeItem*>& highlight);
        void handleWaveAppended(WaveData* wd);
        void handleStateChanged(NetlistSimulatorController::SimulationState state);
        void visualizeCurrentNetState(float tCursor, int xpos);

    Q_SIGNALS:
        void stateChanged(NetlistSimulatorController::SimulationState state);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        NetlistSimulatorController* mController;
        std::unique_ptr<NetlistSimulatorController> mControllerOwner;


        WaveTreeView*        mTreeView;
        WaveTreeModel*       mTreeModel;
        WaveGraphicsCanvas*  mGraphicsCanvas;
        bool                 mOngoingYscroll;
        WaveDataList*        mWaveDataList;
        WaveItemHash*        mWaveItemHash;

        bool mVisualizeNetState;
        bool mAutoAddWaves;
        u32 mGroupIds[3];
    };

}

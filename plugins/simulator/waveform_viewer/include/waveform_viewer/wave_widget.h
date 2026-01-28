// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <QGraphicsView>

#include <QLabel>
#include <QVector>
#include <QMap>
#include <QSplitter>
#include <QFrame>
#include <QPointer>

#include <hal_core/defines.h>
#include "waveform_viewer/wave_item.h"
#include "waveform_viewer/wave_selection_dialog.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"

namespace hal {

    class WaveGraphicsCanvas;
    class WaveTreeModel;
    class WaveTreeView;
    class ModuleItem;

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
        bool hasOwnership() const;
        bool triggerClose();
        void setGates(const std::vector<Gate*>& gats);
        void addResults();
        void addSelectedResults(const QMap<WaveSelectionEntry,int>& sel);
        NetlistSimulatorController::SimulationState state() const;
        void createEngine(const QString& engineFactoryName);
        void refreshNetNames();
        void scrollToYpos(int ypos);
        void removeGroup(u32 grpId);
        bool canImportWires() const;
        bool isEmpty() const;
        WaveGraphicsCanvas* graphicCanvas() { return mGraphicsCanvas; }
        QMap<WaveSelectionEntry,int> addableEntries() const;

    private Q_SLOTS:

        void handleSelectionHighlight(const QVector<const hal::ModuleItem*>& highlight);
        void handleNumberWaveformChanged(int count);
        void handleStateChanged(NetlistSimulatorController::SimulationState state);
        void visualizeCurrentNetState(double tCursor, int xpos);

    Q_SIGNALS:
        void stateChanged(hal::NetlistSimulatorController::SimulationState state);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        QPointer<NetlistSimulatorController> mController;
        std::unique_ptr<NetlistSimulatorController> mControllerOwner;

        u32 mControllerId;
        std::string mControllerName;

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

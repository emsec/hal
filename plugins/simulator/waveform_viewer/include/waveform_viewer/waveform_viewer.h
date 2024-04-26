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

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_writer/netlist_writer.h"
#include "gui/content_widget/content_widget.h"
#include "gui/content_manager/content_manager.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"

#include <functional>
#include <map>
#include <sstream>
#include <QString>
#include <QMap>
#include <QCheckBox>
#include <QTabWidget>
#include <vector>
#include <memory>
#include <unordered_set>

class QStatusBar;
class QProgressBar;
class QCloseEvent;

namespace hal
{
    /* forward declaration */
    class Netlist;
    class WaveWidget;
    class WaveData;
    class Toolbar;
    class SimulationInput;

    class NETLIST_API WaveformViewerFactory : public ContentFactory
    {
    public:
        WaveformViewerFactory(const QString& name) : ContentFactory(name) {;}
        ExternalContentWidget* contentFactory() const;
    };

    /**
     * @ingroup netlist_writer
     */
    class NETLIST_API WaveformViewer : public ExternalContentWidget
    {
        Q_OBJECT

    public:

        WaveformViewer(QWidget* parent = nullptr);
        ~WaveformViewer();

        /**
         * Setups the toolbar with the actions that are supported by the vcd-viewer.
         *
         * @param toolbar - The toolbar to set up
         */
        virtual void setupToolbar(Toolbar* toolbar) override;

        /**
         * Restore previous controller and waveform viewer from saved data files.
         */
        void restoreFromProject() override;
    private Q_SLOTS:
        void handleCreateControl();
        void handleSimulSettings();
        void handleOpenInputFile();
        void handleSaveWaveforms();
        void handleAddResultWave();
        void handleToggleMaxZoom();
        void handleUndoZoomShift();

        void handleSelectGates();
        void handleClockSet();
        void handleEngineSelected(bool checked);
        void handleOpenSettingsDialog();
        void handleShowEngineOutput();

        void handleSelectionChanged(void* sender);
        void handleRefreshNetNames();
        void setVisualizeNetState(bool state);
        void handleTabClosed(int inx);

    public Q_SLOTS:
        void handleControllerAdded(u32 controllerId);
        void handleControllerRemoved(u32 controllerId);
        void handleParseComplete();
        void handleRunSimulation();
        void displayStatusMessage(const QString& msg = QString());
        void closeEvent(QCloseEvent *event) override;
        void currentTabChanged(int inx);
        void currentStateChanged(NetlistSimulatorController::SimulationState state);
        void showProgress(int percent);
        void testUndoEnable();

    private:
        bool mVisualizeNetState;

        QAction* mCreateControlAction;
        QAction* mSimulSettingsAction;
        QAction* mOpenInputfileAction;
        QAction* mSaveWaveformsAction;
        QAction* mRunSimulationAction;
        QAction* mAddResultWaveAction;
        QAction* mToggleMaxZoomAction;
        QAction* mUndoZoomShiftAction;

        QTabWidget* mTabWidget;
        QStatusBar* mStatusBar;
        QProgressBar* mProgress;
        WaveWidget* mCurrentWaveWidget;

        void takeControllerOwnership(std::unique_ptr<NetlistSimulatorController>& ctrlRef, bool create);
        void showCloseButton();
    };
}    // namespace hal

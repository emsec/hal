//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_writer/netlist_writer.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_data.h"

namespace hal {
/* forward declaration */
class Netlist;

/**
 * @ingroup netlist_writer
 */
class NETLIST_API NetlistSimulatorController : public QObject {
    Q_OBJECT

public:
    enum SimulationState { NoGatesSelected,
        ParameterSetup,
        ParameterReady,
        SimulationRun,
        ShowResults,
        EngineFailed };

    NetlistSimulatorController(u32 id, const std::string nam, QObject* parent = nullptr);
    ~NetlistSimulatorController();

    /**
     * Call to one of the registered engine factories to create a new engine.
     * Controller will take ownership for new engine.
     * @param[in] name name of engine factory (thus name of the engine)
     * @return Pointer to engine if successfully created, nullptr otherwise
     */
    SimulationEngine* create_simulation_engine(const std::string& name);

    /**
     * Getter for simulation engine (if any)
     * @return Pointer to engine or nullptr
     */
    SimulationEngine* get_simulation_engine() const;

    /**
     * Specify a net that carries the clock signal and set the clock frequency in hertz.
     * This function can only be called before the simulation has been initialized.
     *
     * @param[in] clock_net - The net that carries the clock signal.
     * @param[in] frequency - The clock frequency in hertz.
     * @param[in] start_at_zero - Initial clock state is 0 if true, 1 otherwise.
     */
    void add_clock_frequency(const Net* clock_net, u64 frequency, bool start_at_zero = true);

    /**
     * Specify a net that carries the clock signal and set the clock period in picoseconds.
     * This function can only be called before the simulation has been initialized.
     *
     * @param[in] clock_net - The net that carries the clock signal.
     * @param[in] period - The clock period from rising edge to rising edge in picoseconds.
     * @param[in] start_at_zero - Initial clock state is 0 if true, 1 otherwise.
     * @param[in] duration - Optional max time limit when showing clock in VCD viewer or editor
     */
    void add_clock_period(const Net* clock_net, u64 period, bool start_at_zero = true, u64 duration=0);

    /**
     * Add gates to the simulation set that contains all gates that are considered during simulation.
     * This function can only be called before the simulation has been initialized.
     *
     * @param[in] gates - The gates to add.
     */
    void add_gates(const std::vector<Gate*>& gates);

    /**
     * Set the signal for a specific wire to control input signals between simulation cycles.
     *
     * @param[in] net - The net to set a signal value for.
     * @param[in] value - The value to set.
     */
    void set_input(const Net* net, BooleanFunction::Value value);

    /**
     * Initialize the simulation.
     * No additional gates or clocks can be added after this point.
     */
    void initialize();

    /**
     * Simulate for a specific period, advancing the internal state.
     * Automatically initializes the simulation if 'initialize' has not yet been called.
     * Use 'set_input' to control specific signals.
     *
     * @param[in] picoseconds - The duration to simulate.
     */
    void simulate(u64 picoseconds);

    /**
     * Reset the simulator state, i.e., treat all signals as unknown.
     * Does not remove gates/nets from the simulation set.
     */
    void reset();

    /**
     * Shortcut to SimulationInput::get_gates
     */
    const std::unordered_set<const Gate*>& get_gates() const;

    /**
     * Shortcut to SimulationInput::get_input_nets
     */
    const std::unordered_set<const Net*>& get_input_nets() const;

    /**
     * Shortcut to SimulationInput::get_output_nets
     */
    const std::vector<const Net*>& get_output_nets() const;

    /**
     * Shortcut to SimulationEngines::instance()->names()
     * @return names of registered simulation engines
     */
    std::vector<std::string> get_engine_names() const;

    //        SimulationInput* input() const { return mSimulationInput; }

    /**
     * Getter for controller name
     * @return name as QString
     */
    QString name() const { return mName; }

    /**
     * Getter for controller name
     * @return name as std::string
     */
    std::string get_name() const { return mName.toStdString(); }

    /**
     * Getter for controller ID
     * @return the ID
     */
    u32 get_id() const { return mId; }

    /**
     * Get simulated data from engine, either from shared memory or from VCD file
     * @return true on success, false otherwise
     */
    bool get_results();

    /**
     * run simulation
     * @return true on success, false otherwise
     */
    bool run_simulation();

    /**
     * Request to engine to generate a VCD file for simulated netlist.
     * @param[in] filename requested filename to be generated by engine
     */
    void request_generate_vcd(const std::string& filename);

    /**
     * Parse VCD file and set wave data
     * @param[in] filename the filename to read
     */
    void parse_vcd(const std::string& filename);

    /**
     * Generates the a partial VCD file for parts the simulated netlist.
     *
     * @param[in] path - The path to the VCD file.
     * @param[in] start_time - Start of the timeframe to write to the file (in picoseconds).
     * @param[in] end_time - End of the timeframe to write to the file (in picoseconds).
     * @param[in] nets - Nets to include in the VCD file.
     * @returns True if the file gerneration was successful, false otherwise.
     */
    bool generate_vcd(const std::filesystem::path& path, u32 start_time=0, u32 end_time=0, std::set<const Net*> nets = {}) const;

    /**
     * Getter for wave data list - simulation input as well as output
     * @return const pointer to wave data list
     */
    WaveDataList* get_waves() const { return mWaveDataList; }

    /**
     * Getter for controller state
     * @return state of type SimulationState
     */
    SimulationState get_state() const { return mState; }

public Q_SLOTS:
    void handleSimulSettings();
    void handleOpenInputFile(const QString& filename);
    void handleRunSimulation();
    void handleSelectGates();
    void handleRunFinished(bool success);

Q_SIGNALS:
    void stateChanged(SimulationState state);

private:
    void initSimulator();
    void setState(SimulationState stat);

    bool isClockSet() const;
    bool isInputSet() const;
    void checkReadyState();

    u32 mId;
    QString mName;

    SimulationState mState;
    SimulationEngine* mSimulationEngine;

    WaveDataList* mWaveDataList;

    SimulationInput* mSimulationInput;
};

class NetlistSimulatorControllerMap : public QObject {
    Q_OBJECT

    QMap<u32, NetlistSimulatorController*> mMap;
    NetlistSimulatorControllerMap() { ; }
    static NetlistSimulatorControllerMap* sInst;

Q_SIGNALS:
    void controllerAdded(u32 id);
    void controllerRemoved(u32 id);

public:
    static NetlistSimulatorControllerMap* instance();
    void addController(NetlistSimulatorController* ctrl);
    void removeController(u32 id);
    NetlistSimulatorController* controller(u32 id) const { return mMap.value(id); }
};
} // namespace hal

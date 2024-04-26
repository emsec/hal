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
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_writer/netlist_writer.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QDir>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_data.h"

class QTemporaryDir;

namespace hal {
/* forward declaration */
class Netlist;

/**
 * @ingroup netlist_writer
 */
class NETLIST_API NetlistSimulatorController : public QObject {
    Q_OBJECT

public:
    enum SimulationState {
        NoGatesSelected,
        ParameterSetup,
        ParameterReady,
        SimulationRun,
        ShowResults,
        EngineFailed };

    enum FilterInputFlag {
        GlobalInputs,
        PartialNetlist,
        CompleteNetlist,
        NoFilter
    };

    NetlistSimulatorController(u32 id, const std::string nam, const std::string& workdir, QObject* parent = nullptr);

    NetlistSimulatorController(u32 id, Netlist* nl, const std::string& filename, QObject* parent = nullptr);

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
     * Prepare simulation where no net is defined as clock input
     */
    void set_no_clock_used();

    /**
     * Add waveform group. Netlist must not be empty. First net in list is considered the lowest significant bit.
     * @param name The waveform group name
     * @param nets List of nets for group
     * @return ID of new waveform group
     */
    u32 add_waveform_group(const std::string& name, const std::vector<Net*>& nets);

    /**
     * Create a waveform group from the nets of a given module pin group.
     * 
     * @param pin_group The pin_group to create waveform group from.
     * @return ID of new waveform group
     */
    u32 add_waveform_group(const PinGroup<ModulePin>* pin_group);


    /**
     * Create a waveform group from the nets of a given module pin group.
     * 
     * @param pin_group The pin_group to create waveform group from.
     * @param name The name of the newly created waveform group from.
     * @return ID of new waveform group
     */
    u32 add_waveform_group(const std::string& name, const PinGroup<ModulePin>* pin_group);

    /**
     * Add waveform based on boolean expression.
     * @param expression The boolean expression based on names of existing waveforms.
     * @return ID of new boolean waveform
     */
    u32 add_boolean_expression_waveform(const std::string& expression);

    /**
     * Add boolean waveform based on list of accepted combinations
     * @param input_waves List of input waveforms
     * @param accept_combination List of accepted combinations. Each combination is coded as binary integer value which is matched bitwise with value of input waveform.
     * @return ID of new boolean waveform
     */
    u32 add_boolean_accept_list_waveform(const std::vector<WaveData*>& input_waves, const std::vector<int>& accepted_combination);

    /**
     * Add trigger time vector based on wave transitions.
     * @param trigger_waves List of source waveform to generate trigger time
     * @param trigger_on_values Optional list of values. Order of values must match order of waveform. Trigger will only be generated upon transition to value.
     * @return ID of new trigger time
     */
    u32 add_trigger_time(const std::vector<WaveData*>& trigger_waves, const std::vector<int>& trigger_on_values = std::vector<int>());

    /**
     * Remove waveform group identified by group ID. Waveform for nets will still be shown but they are not bundled.
     * @param group_id The ID of waveform group to be removed
     */
    void remove_waveform_group(u32 group_id);

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
     * Set the signal for a specific wire to control input signals between simulation cycles.
     *
     * @param[in] wd - WaveData object of the signal.
     * @param[in] value - The value to set.
     */
    void set_input(WaveData* wd, BooleanFunction::Value value);

    /**
     * Set the signal for a group of nets to control input signals between simulation cycles.
     *
     * @param[in] nets - The vector of nets. 
     * @param[in] values - The values to set.
     */
    void set_input(const std::vector<Net*>& nets, const std::vector<BooleanFunction::Value>& values);

    /**
     * Set the signals for a specific WaveDataGroup to control input signals between simulation cycles.
     *
     * @param[in] wdg - The WaveDataGroup.
     * @param[in] values - The values to set.
     */
    void set_input(const WaveDataGroup* wdg, const std::vector<BooleanFunction::Value>& values);

    /**
     * Set the signals for a specific WaveDataGroup to control input signals between simulation cycles.
     *
     * @param[in] id - ID of the WaveDataGroup.
     * @param[in] values - The values to set.
     */
    void set_input(const u32 id, const std::vector<BooleanFunction::Value>& values);

    /**
     * Set the signals for a specific module pin group to control input signals between simulation cycles.
     *
     * @param[in] pin_group - The module pin group.
     * @param[in] values - The values to set.
     */
    void set_input(const PinGroup<ModulePin>* pin_group, const std::vector<BooleanFunction::Value>& values);

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
     * Shortcut to SimulationInput::get_partial_netlist_nets
     */
    const std::vector<const Net*>& get_partial_netlist_nets() const;

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
     * Convenience method to get the last time covered by simulation. This is the last transition found in waveforms.
     * @return Maximum of simulated time
     */
    u64 get_max_simulated_time() const;

    /**
     * run simulation
     * @return true on success, false otherwise
     */
    bool run_simulation();

    /**
     * Send signal to viewer to start simulation.
     */
    void emit_run_simulation();

    /**
     * Import VCD file and convert content into SALEAE format
     * @param[in] filename the filename to read
     * @param[in] filter filter to select waveform data from file
     */
    bool import_vcd(const std::string& filename, FilterInputFlag filter);

    /**
     * Import CSV file and convert content into SALEAE format
     * @param[in] filename the filename to read
     * @param[in] filter filter to select waveform data from file
     * @param[in] timescale multiplication factor for time value in first column
     */
    void import_csv(const std::string& filename, FilterInputFlag filter, u64 timescale = 1000000000);

    /**
     * Import nets given by lookup table from SALEAE directory
     * @param[in] dirname the directory to import files from
     * @param[in] lookupTable mapping nets to be imported with saleae file index
     * @param[in] timescale multiplication factor for time value if SALEAE data in float format
     */
    void import_saleae(const std::string& dirname, std::unordered_map<Net*,int> lookupTable, u64 timescale = 1000000000);

    /**
     * Imports nets from simulation working directory. Existing saleae directory required to nets with binary data
     * @param[in] dirname the directory to import files from
     * @param[in] filter filter to select waveform data for import
     * @param[in] timescale multiplication factor for time value if SALEAE data in float format
     */
    void import_simulation(const std::string& dirname, FilterInputFlag filter, u64 timescale = 1000000000);

    /**
     * Set timescale when parsing SALEAE float values
     * @param[in] timescale multiplication factor for time value
     */
    void set_saleae_timescale(u64 timescale = 1000000000);

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
     * The working directory. Directory is temporary and will be removed when controller gets deleted
     * @return directory path
     */
    std::string get_working_directory() const;

    /**
     * Verilator's gmake disallows spaces in directory name. Other tests (e.g. dot) might be added in future.
     * @return true if directory name is legal, false otherwise
     */
    bool is_legal_directory_name() const;

    /**
     * Get the filename of SALEAE directory file (JSON format).
     * @return filename as std path
     */
    std::filesystem::path get_saleae_directory_filename() const;

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

    /**
     * Getter for a single waveform
     * @param[in] Pointer to net waveform is associated with
     * @return The waveform data
     */
    WaveData* get_waveform_by_net(const Net* n) const;

    /**
     * Rename waveform and emit 'renamed' signal
     * @param wd[in] Waveform to be renamed
     * @param name[in] New name for waveform
     */
    void rename_waveform(WaveData* wd, std::string name);

    /**
     * Getter for waveform group
     * @param id[in] Waveform group id
     * @return The waveform group object
     */
    WaveDataGroup* get_waveform_group_by_id(u32 id) const;

    /**
     * Getter for boolean waveform (which is a combination of several other waveform by boolean operation).
     * @param id[in] Boolean waveform id
     * @return The boolean waveform object
     */
    WaveDataBoolean* get_waveform_boolean_by_id(u32 id) const;

    /**
     * Getter for trigger time set.
     * @param id[in] Trigger time id
     * @return The trigger time object which derives from WaveData.
     */
    WaveDataTrigger* get_trigger_time_by_id(u32 id) const;

    /**
     * Set timeframe for viewer
     * @param[in] tmin Lower limit for time scale in wave viewer
     * @param[in] tmax Upper limit for time scale in wave viewer
     */
    void set_timeframe(u64 tmin=0, u64 tmax=0);

    /**
     * Emit load progress when importing VCD, CSV, or SALEAE waveform
     * @param[in] percent load progress, negative values to hide progess bar
     */
    void emitLoadProgress(int percent);

    /**
     * Controller is in a state that allows import from VCD, CSV, or SALEAE waveform data
     * @return True if import is allowed, false otherwise.
     */
    bool can_import_data() const;

    /**
     * Store significant information into working directory
     * @return True if JSON file created successfully, false otherwise.
     */
    bool persist() const;

    static const char* sPersistFile;

public Q_SLOTS:
    void handleOpenInputFile(const QString& filename);
    void handleSelectGates();
    void handleRunFinished(bool success);

Q_SIGNALS:
    void stateChanged(hal::NetlistSimulatorController::SimulationState state);
    void engineFinished(bool success);
    void parseComplete();
    void loadProgress(int percent);
    void triggerRunSimulation();

private:
    std::vector<const Net*> getFilterNets(FilterInputFlag filter) const;
    void initSimulator();
    void setState(SimulationState stat);
    bool getResultsInternal();

    bool isClockSet() const;
    bool isInputSet() const;
    void checkReadyState();
    void restoreComposed(const SaleaeDirectory& sd);
    void loadStoredController(const QDir& workDir);

    u32 mId;
    QString mName;

    SimulationState mState;
    SimulationEngine* mSimulationEngine;

    QTemporaryDir* mTempDir;
    QString mWorkDir;
    WaveDataList* mWaveDataList;

    SimulationInput* mSimulationInput;

    QHash<u32,int> mBadAssignInputWarnings;
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
    void shutdown() { mMap.clear(); }
    QList<NetlistSimulatorController*> toList() const { return mMap.values(); }
    NetlistSimulatorController* controller(u32 id) const { return mMap.value(id); }
};
} // namespace hal

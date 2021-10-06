#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/defines.h"

namespace hal {

    class SimulationEngine {
        std::string mName;
    protected:
        bool mRequireClockEvents;
        bool mCanShareMemory;
        std::string mResultFilename;
    public:
        SimulationEngine(const std::string& nam);
        virtual ~SimulationEngine() {;}

        /**
         * Request clock change as regular net input event
         * @return true if clock events are required by engine, false otherwise
         */
        bool clock_events_required() const {return mRequireClockEvents; }

        /**
         * Tells the caller whether engine can share simulation results directly from memory.
         * If not the caller will most likely ask to have a VCD result file created.
         * @return true if results can be read from memory
         */
        bool can_share_memory() const { return mCanShareMemory; }

        /**
         * Must be implemented by derived class
         *
         * Will be called by controller to pass all relevant information to setup the simulation
         * @param[in] simInput the input
         * @return true if engine could be prepared successfully with given input, false on error
         */
        virtual bool setSimulationInput(SimulationInput* simInput) = 0;

        /**
         * Set the name of VCD result file. If set engine is requested to produce such a file upon finalize
         * @param[in] the name of the file to be created.
         */
        virtual void setResultFilename(const std::string filename) {mResultFilename = filename; };

        /**
         * Must be implemented by derived class
         *
         * Will be called by controller to start simulation.
         *
         * Derived classes SimulationEngineEventDriven and SimulationEngineScripted will
         * implement their own run handlers to start either a separate process or a thread
         *
         * @return true if successful, false on error
         */
        virtual bool run() = 0;

        /**
         * Can be implemented by derived class
         *
         * Signals the engine that it is time for final steps (.e.g. writing VCD file)
         * after simulation is done - that is:
         * SimulationEngineEventDriven:   all input events have been processed
         * SimulationEngineScripted:      all comands executed successfully
         *
         * @return true if successful, false on error
         */
        virtual bool finalize() { return true; }

        std::string name() const { return mName; }
    };

    class SimulationEngineEventDriven : public SimulationEngine
    {
        bool run() override;
        bool setSimulationInput(SimulationInput *simInput) override;
    protected:
        SimulationInput* mSimulationInput;
    public:
        SimulationEngineEventDriven(const std::string& nam);

        /**
         * Must be implemented by derived class
         *
         * Passes an event to engine to trigger simulation
         * @param[in] netEv input event asigning input values (0,1,X,Z) for nets and providing number of cycles to simulate
         * @return true if event was handled successfully, false otherwise
         */
        virtual bool inputEvent(const SimulationInputNetEvent& netEv) = 0;

        /**
         * Get vector of simulated events for net
         *
         * @param[in] n - The net for which events where simulated
         * @return Vector of events
         */
        virtual std::vector<WaveEvent> get_simulation_events(Net *n) const;
    };

    class SimulationEngineScripted : public SimulationEngine
    {
        virtual bool run() override;
    public:
        SimulationEngineScripted(const std::string& nam) : SimulationEngine(nam) {;}

        /**
         * Must be implemented by derived class
         *
         * Returns the information how many
         * command lines are to be executed
         * @return number of command lines
         */
        virtual int numberCommandLines() const = 0;

        /**
         * Must be implemented by derived class
         *
         * Returns command and argument to be executed
         * @param[in] lineNumber the line index of the script starting from 0
         * @return first element is command to be executed, subsequent elements are arguments
         */
        virtual std::vector<std::string> commandLine(int lineIndex) const = 0;
    };

    class SimulationEngines : public std::vector<SimulationEngine*>
    {
        SimulationEngines() {;}
        static SimulationEngines* inst;
    public:
        static SimulationEngines* instance();

        std::vector<std::string> names() const;
        SimulationEngine* engineByName(const std::string nam) const;
        void deleteEngine(const std::string nam);
    };
}

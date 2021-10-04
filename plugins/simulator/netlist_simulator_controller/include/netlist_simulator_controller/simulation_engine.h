#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/defines.h"

namespace hal {

    class SimulationEngine {
        std::string mName;
    public:
        SimulationEngine(const std::string& nam);

        /**
         * Must be implemented by derived class
         *
         * Will be called by controller to pass all relevant information to setup the simulation
         * @param[in] simInput the input
         */
        virtual void setSimulationInput(SimulationInput* simInput) = 0;

        /**
         * Must be implemented by derived class
         *
         * Returns VCD file name of results if any.
         * @return file name or empty string if no VCD file has been generated (yet).
         */
        virtual std::string resultFilename() const = 0;

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
         * Signals the engine that simulation is done
         * SimulationEngineEventDriven:   all input events have been processed
         * SimulationEngineScripted:      all comands executed successfully
         */
        virtual void done() {;}

        std::string name() const { return mName; }
    };

    class SimulationEngineEventDriven : public SimulationEngine
    {
        bool run() override;
        void setSimulationInput(SimulationInput *simInput) override { mSimulationInput = simInput; }
    protected:
        SimulationInput* mSimulationInput;
    public:
        SimulationEngineEventDriven(const std::string& nam) : SimulationEngine(nam), mSimulationInput(nullptr) {;}

        /**
         * Must be implemented by derived class
         *
         * Passes an event to engine to trigger simulation
         * @param[in] netEv input event asigning input values (0,1,X,Z) for nets and providing number of cycles to simulate
         * @return true if event was handled successfully, false otherwise
         */
        virtual bool inputEvent(const SimulationInputNetEvent& netEv) = 0;
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
    };
}

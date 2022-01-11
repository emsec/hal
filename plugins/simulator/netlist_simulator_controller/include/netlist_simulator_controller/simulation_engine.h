#pragma once
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_event.h"

#include <string>
#include <unordered_map>
#include <vector>

class QTemporaryDir;

namespace hal
{
    class NetlistSimulatorController;

    class SimulationEngine
    {
        std::string mName;

    public:
        enum State
        {
            Failed    = -1,
            Done      = 0,
            Running   = 1,
            Preparing = 2
        };

    protected:
        bool mRequireClockEvents;
        bool mCanShareMemory;
        std::string mResultFilename;
        State mState;
        std::unordered_map<std::string, std::string> mProperties;
        QTemporaryDir* mTempDir;
        SimulationInput* mSimulationInput;

    public:
        SimulationEngine(const std::string& nam);
        virtual ~SimulationEngine();

        /**
         * Getter for enging name
         * @return name as std::string
         */
        std::string name() const
        {
            return mName;
        }

        /**
         * State of the engine
         * @return possible state values are Preparing, Running, Done, Failed
         */
        State state() const
        {
            return mState;
        }

        /**
         * State of the engine as integer
         * @return possible state values are Preparing = 2, Running = 1, Done = 0, Failed = -1
         */
        int get_state() const
        {
            return mState;
        }

        /**
         * The working directory. Directory is temporary and will be removed when engine gets deleted
         * @return directory path
         */
        std::string directory() const;

        /**
         * Request clock change as regular net input event
         * @return true if clock events are required by engine, false otherwise
         */
        bool clock_events_required() const
        {
            return mRequireClockEvents;
        }

        /**
         * Tells the caller whether engine can share simulation results directly from memory.
         * If not the caller will most likely ask to have a VCD result file created.
         * @return true if results can be read from memory
         */
        bool can_share_memory() const
        {
            return mCanShareMemory;
        }

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
        virtual void setResultFilename(const std::string filename)
        {
            mResultFilename = filename;
        };

        /**
         * Getter for file name of results as VCD
         * @return the file name
         */
        std::string get_result_filename() const
        {
            return mResultFilename;
        }

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
        virtual bool run(NetlistSimulatorController* controller) = 0;

        /**
         * Can be overwritten by derived class
         *
         * Signals the engine that it is time for final steps (.e.g. writing VCD file)
         * after simulation is done - that is:
         * SimulationEngineEventDriven:   all input events have been processed
         * SimulationEngineScripted:      all comands executed successfully
         *
         * @return true if successful, false on error
         */
        virtual bool finalize();

        /**
         * Can be overwritten by derived class
         *
         * Signals the engine that an essential step failed and execution is going to be
         * aborted. Engine might want to do some final clean up.
         */
        virtual void failed();

        /**
         * Set property which can be evaluated by engine
         *
         * @param key property name
         * @param value property value
         */
        virtual void set_engine_property(const std::string& key, const std::string& value);
        
        
        /**
         * Set property which can be evaluated by engine
         *
         * @param key property name
         * @param value property value
         */
        virtual std::string get_engine_property(const std::string& key);

        /**
         * Get simulation input
         * @return Pointer to input instance
         */
        virtual SimulationInput* get_simulation_input() const { return mSimulationInput; }
    };

    class SimulationEngineEventDriven : public SimulationEngine
    {
        bool run(NetlistSimulatorController* controller) override;
        bool setSimulationInput(SimulationInput* simInput) override;

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
        virtual std::vector<WaveEvent> get_simulation_events(u32 netId) const;
    };

    class SimulationEngineScripted : public SimulationEngine
    {
        virtual bool run(NetlistSimulatorController* controller) override;

    public:
        SimulationEngineScripted(const std::string& nam) : SimulationEngine(nam)
        {
            ;
        }

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

    class SimulationEngineFactory
    {
    protected:
        std::string mName;

    public:
        SimulationEngineFactory(const std::string& nam);
        virtual ~SimulationEngineFactory()
        {
            ;
        }
        virtual SimulationEngine* createEngine() const = 0;
        std::string name() const
        {
            return mName;
        }
    };

    class SimulationEngineFactories : public std::vector<SimulationEngineFactory*>
    {
        SimulationEngineFactories()
        {
            ;
        }
        static SimulationEngineFactories* inst;

    public:
        static SimulationEngineFactories* instance();

        std::vector<std::string> factoryNames() const;
        SimulationEngineFactory* factoryByName(const std::string nam) const;
        void deleteFactory(const std::string nam);
    };
}    // namespace hal

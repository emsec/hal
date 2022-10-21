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
#include "hal_core/netlist/boolean_function.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_event.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

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
        SimulationInput* mSimulationInput;
        std::string mWorkDir;

    public:
        SimulationEngine(const std::string& nam);
        virtual ~SimulationEngine() {;}

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
         * The working directory. Directory is temporary and will be removed when controller gets deleted
         * @return directory path
         */
        std::string get_working_directory() const;

        /**
         * Set the working directory. Method should be called by controller only after creating engine instance.
         * @param[in] workDir The working directory (should be controller tmp directory)
         */
        void set_working_directory(const std::string& workDir);

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
         * Copy header and source files so that saleae parser can be compiled into project
         * @param dirname[in] Target directory to copy files into
         * @return true if files copied successfully, false otherwise
         */
        bool install_saleae_parser(std::string dirname) const;

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
         * Get all propertiess
         * @return
         */
        virtual const std::unordered_map<std::string,std::string>& get_engine_properties() const { return mProperties; }

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

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
#include <vector>
#include <string>
#include <unordered_map>
#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/defines.h"

namespace hal {

    class DummyEngine : public SimulationEngineScripted
    {
    public:
        DummyEngine(const std::string& nam) : SimulationEngineScripted(nam) {;}

        virtual bool setSimulationInput(SimulationInput* simInput);
        int numberCommandLines() const;
        std::vector<std::string> commandLine(int lineIndex) const;
        bool finalize();
    };

    class DummyEngineFactory : SimulationEngineFactory
    {
        static DummyEngineFactory* sDumFac;
    public:
        DummyEngineFactory() : SimulationEngineFactory("dummy_engine") {;}
        SimulationEngine* createEngine() const override { return new DummyEngine(mName); }
    };
}

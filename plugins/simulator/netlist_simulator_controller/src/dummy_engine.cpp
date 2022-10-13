#include "netlist_simulator_controller/dummy_engine.h"
#include <QDebug>

namespace hal {
    DummyEngineFactory* DummyEngineFactory::sDumFac = new DummyEngineFactory;

    bool DummyEngine::setSimulationInput(SimulationInput* simInput)
    {
        mSimulationInput = simInput;
        simInput->dump( (get_working_directory() + "/simulation_input.txt").c_str() );
        return true;
    }

    int DummyEngine::numberCommandLines() const
    {
        return 1;
    }

    std::vector<std::string> DummyEngine::commandLine(int lineIndex) const
    {
        std::vector<std::string> retval;
        if (lineIndex) return retval;

        const char* cl[] = { "/usr/bin/touch", "dummy", nullptr };
        for (int i = 0; cl[i]; i++)
            retval.push_back(std::string(cl[i]));

        return retval;
    }

    bool DummyEngine::finalize()
    {
        qDebug() << "DummyEngine::finalize";
        mState = Done;
        return true;
    }

}

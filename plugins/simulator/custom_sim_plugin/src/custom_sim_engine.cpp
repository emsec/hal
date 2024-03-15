#include "custom_sim_plugin/custom_sim_engine.h"
#include <QDebug>

namespace hal {

    CustomSimEngineFactory* CustomSimEngineFactory::sDumFac = new CustomSimEngineFactory;

    bool CustomSimEngine::setSimulationInput(SimulationInput* simInput)
    {
        mSimulationInput = simInput;
        simInput->dump( (get_working_directory() + "/simulation_input.txt").c_str() );
        return true;
    }

    int CustomSimEngine::numberCommandLines() const
    {
        return 1;
    }

    std::vector<std::string> CustomSimEngine::commandLine(int lineIndex) const
    {
        std::vector<std::string> retval;
        if (lineIndex) return retval;

        const char* cl[] = { "/usr/bin/touch", "dummy", nullptr };
        for (int i = 0; cl[i]; i++)
            retval.push_back(std::string(cl[i]));

        return retval;
    }

    bool CustomSimEngine::finalize()
    {
        qDebug() << "CustomSimEngine::finalize";
        mState = Done;
        return true;
    }

    SimulationEngine* CustomSimEngineFactory::createEngine() const {
        return new CustomSimEngine(mName);
    }
}

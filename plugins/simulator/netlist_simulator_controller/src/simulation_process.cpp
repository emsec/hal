#include "netlist_simulator_controller/simulation_process.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include <QProcess>
#include <QThread>
#include <vector>
#include <QStringList>

namespace hal {

    SimulationProcess::SimulationProcess(NetlistSimulatorController *controller, SimulationEngineScripted *engine)
        : mEngine(engine), mLineIndex(0), mNumberLines(0)
    {
        connect(this, &SimulationProcess::processFinished, controller, &NetlistSimulatorController::handleRunFinished);
    }


    void SimulationProcess::start()
    {
        mNumberLines = mEngine->numberCommandLines();
        launchProcess();
    }

    void SimulationProcess::handleStepFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        if (exitStatus != QProcess::NormalExit || exitCode != 0)
        {
            Q_EMIT processFinished(false);
            return;
        }
        ++mLineIndex;
        if (mLineIndex >= mNumberLines)
        {
            Q_EMIT processFinished(true);
            return;
        }
        launchProcess();
    }

    bool SimulationProcess::launchProcess()
    {
        if (mLineIndex >= mNumberLines) return false;

        QStringList args;
        QString prog;

        bool first = true;
        for (const std::string s : mEngine->commandLine(mLineIndex))
        {
            QString qs = QString::fromStdString(s);
            if (first)
                prog = qs;
            else
                args << qs;
            first = false;
        }
        if (prog.isEmpty()) return false;

        mProcess = new QProcess(this);
        mProcess->setWorkingDirectory(QString::fromStdString(mEngine->directory()));
        connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &SimulationProcess::handleStepFinished);
        mProcess->start(prog, args);

        return true;
    }
}

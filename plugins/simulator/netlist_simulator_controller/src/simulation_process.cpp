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

    void SimulationProcess::abortOnError()
    {
        mEngine->failed();
        Q_EMIT processFinished(false);
    }

    void SimulationProcess::run()
    {
        mNumberLines = mEngine->numberCommandLines();
        while (mLineIndex < mNumberLines)
        {

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
            if (prog.isEmpty())
                return abortOnError();

            mProcess = new QProcess;
            mProcess->setWorkingDirectory(QString::fromStdString(mEngine->directory()));
            mProcess->start(prog, args);

            if (!mProcess->waitForStarted())
                return abortOnError();

            if (!mProcess->waitForFinished(-1))
                return abortOnError();

            if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0)
                return abortOnError();

            ++ mLineIndex;
        }

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);
    }
}

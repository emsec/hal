#include "netlist_simulator_controller/simulation_process.h"
#include <QProcess>
#include <QThread>
#include <vector>
#include <QStringList>

namespace hal {

    SimulationProcess::SimulationProcess(SimulationEngineScripted *engine)
        : mEngine(engine), mLineIndex(0), mNumberLines(0) {;}


    void SimulationProcess::start()
    {
        mNumberLines = mEngine->numberCommandLines();
        launchProcess();
    }

    void SimulationProcess::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        Q_UNUSED(exitCode);
        if (exitStatus != QProcess::NormalExit) return;
        ++mLineIndex;
        if (mLineIndex >= mNumberLines) return;
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
        connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &SimulationProcess::handleProcessFinished);
        mProcess->start(prog, args);

        return true;
    }
}

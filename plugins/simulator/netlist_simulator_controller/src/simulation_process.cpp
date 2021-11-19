#include "netlist_simulator_controller/simulation_process.h"

#include "netlist_simulator_controller/netlist_simulator_controller.h"

#include <QProcess>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace hal
{

    SimulationProcess::SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine) : mEngine(engine), mLineIndex(0), mNumberLines(0)
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
        if (mEngine->get_engine_property("server_ip").empty())
        {
            runLocal();
        }
        else
        {
            runRemote();
        }
    }

    bool SimulationProcess::createServerScript()
    {
        std::stringstream simulation_commands;
        for (int i = 0; i < mEngine->numberCommandLines(); i++)
        {
            std::vector<std::string> commands = mEngine->commandLine(i);
            for (const auto& command : commands)
            {
                simulation_commands << command << " ";
            }
            simulation_commands << std::endl;
        }

        std::filesystem::path simulation_commands_path = mEngine->directory();
        simulation_commands_path /= std::string("execute_testbench.sh");

        std::ofstream simulation_commands_file(simulation_commands_path);
        simulation_commands_file << simulation_commands.str();
        simulation_commands_file.close();

        return true;
    }

    bool SimulationProcess::compressSimulationFiles()
    {
        return true;
    }

    bool SimulationProcess::sendFilesToServer()
    {
        return true;
    }

    bool SimulationProcess::loadVCDFromServer()
    {
        return true;
    }

    bool SimulationProcess::deleteJobFromServer()
    {
        return true;
    }

    void SimulationProcess::runRemote()
    {
        // (0) write all commands to script
        if (!createServerScript())
        {
            return abortOnError();
        }

        // (1) tar gz files
        if (!compressSimulationFiles())
        {
            return abortOnError();
        }

        // (2) send file to server
        if (!sendFilesToServer())
        {
            return abortOnError();
        }

        // (3) get vcd file back
        if (!loadVCDFromServer())
        {
            return abortOnError();
        }

        // (4) delete job on server
        if (!deleteJobFromServer())
        {
            return abortOnError();
        }

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);

        // mNumberLines = mEngine->numberCommandLines();
        // while (mLineIndex < mNumberLines)
        // {
        //     QStringList args;
        //     QString prog;

        //     bool first = true;
        //     for (const std::string s : mEngine->commandLine(mLineIndex))
        //     {
        //         QString qs = QString::fromStdString(s);
        //         if (first)
        //             prog = qs;
        //         else
        //             args << qs;
        //         first = false;
        //     }
        //     if (prog.isEmpty())
        //         return abortOnError();

        //     QTextStream xout(stdout, QIODevice::WriteOnly);
        //     int count = 0;
        //     for (const std::string& s : mEngine->commandLine(mLineIndex))
        //     {
        //         xout << QString("engine command line[%1] <%2>").arg(count++).arg(s.c_str()) << "\n";
        //         xout.flush();
        //     }

        //     mProcess = new QProcess;
        //     mProcess->setWorkingDirectory(QString::fromStdString(mEngine->directory()));
        //     mProcess->start(prog, args);

        //     if (!mProcess->waitForStarted())
        //         return abortOnError();

        //     bool success = mProcess->waitForFinished(-1);

        //     xout << "process stdout......\n";
        //     xout << mProcess->readAllStandardOutput();
        //     xout << "\n--------------------\n";
        //     xout << "process stderr......\n";
        //     xout << mProcess->readAllStandardError();
        //     xout << "\n--------------------\n";

        //     if (!success)
        //         return abortOnError();

        //     if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0)
        //         return abortOnError();

        //     ++mLineIndex;
        // }

        // if (!mEngine->finalize())
        //     return abortOnError();

        // Q_EMIT processFinished(true);
    }

    void SimulationProcess::runLocal()
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

            QTextStream xout(stdout, QIODevice::WriteOnly);
            int count = 0;
            for (const std::string& s : mEngine->commandLine(mLineIndex))
            {
                xout << QString("engine command line[%1] <%2>").arg(count++).arg(s.c_str()) << "\n";
                xout.flush();
            }

            mProcess = new QProcess;
            mProcess->setWorkingDirectory(QString::fromStdString(mEngine->directory()));
            mProcess->start(prog, args);

            if (!mProcess->waitForStarted())
                return abortOnError();

            bool success = mProcess->waitForFinished(-1);

            xout << "process stdout......\n";
            xout << mProcess->readAllStandardOutput();
            xout << "\n--------------------\n";
            xout << "process stderr......\n";
            xout << mProcess->readAllStandardError();
            xout << "\n--------------------\n";

            if (!success)
                return abortOnError();

            if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0)
                return abortOnError();

            ++mLineIndex;
        }

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);
    }

}    // namespace hal

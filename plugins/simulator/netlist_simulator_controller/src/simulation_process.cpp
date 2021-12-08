#include "netlist_simulator_controller/simulation_process.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include <QProcess>
#include <QThread>
#include <vector>
#include <QStringList>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

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
        if (mEngine->get_engine_property("ssh_server").empty())
            runLocal();
        else
            runRemote();
    }

    void SimulationProcess::runRemote()
    {
        QDir localDir(QString::fromStdString(mEngine->directory()));
        QString remoteDir = localDir.absolutePath().mid(1);
        QString shellScriptName(localDir.absoluteFilePath("remote.sh"));
        QString hostname = QString::fromStdString(mEngine->get_engine_property("ssh_server"));
        QString resultFile("waveform.vcd");
        QTextStream xout(stdout, QIODevice::WriteOnly);
        xout << "ssh_server   <" << hostname << ">\n";
        xout << "result_file  <" << resultFile << ">\n";
        xout << "local_dir    <" << localDir.absolutePath() << ">\n";
        xout << "remote_dir   <" << remoteDir << ">\n";
        xout << "script       <" << shellScriptName << ">\n";
        QFile ff(shellScriptName);
        if (!ff.open(QIODevice::WriteOnly))
        {
            qDebug() << "cannot open remote script for writing" << shellScriptName;
            return abortOnError();
        }
        ff.write("set -x\n");
        ff.write(QString("tar -czf - %1 | ssh %2 tar -xzf -\n").arg(localDir.absolutePath()).arg(hostname).toUtf8());
        mNumberLines = mEngine->numberCommandLines();
        for (int i=0; i<mNumberLines; i++)
        {
            QString remoteCmd = QString("ssh %1 'cd %2 ;").arg(hostname).arg(remoteDir);
            for (const std::string s : mEngine->commandLine(i))
            {
                remoteCmd += " " + QString::fromStdString(s);
            }
            remoteCmd +=  QString("'\n");
            ff.write(remoteCmd.toUtf8());
        }
        ff.write(QString("ssh %1 'cd %2 ; cat %3' > %4\n").arg(hostname).arg(remoteDir).arg(resultFile).arg(localDir.absoluteFilePath(resultFile)).toUtf8());
        ff.close();
        ff.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);

        mProcess = new QProcess;
        mProcess->setWorkingDirectory(localDir.absolutePath());
        QStringList args;
        args << shellScriptName;
        QString prog("/usr/bin/bash");
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

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);
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

            ++ mLineIndex;
        }

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);
    }
}

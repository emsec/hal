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
#include <QUuid>

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
        QString remoteDir = "hal_simul_" + QUuid::createUuid().toString(QUuid::Id128);
        QString shellScriptName(localDir.absoluteFilePath("remote.sh"));
        QString hostname = QString::fromStdString(mEngine->get_engine_property("ssh_server"));
        QString resultFile("waveform.vcd");
        QFile ff(shellScriptName);
        if (!ff.open(QIODevice::WriteOnly))
        {
            qDebug() << "cannot open remote script for writing" << shellScriptName;
            return abortOnError();
        }
        ff.write("set -x\n");  // echo commands
        ff.write("set -e\n");  // bailout on error
        ff.write(QString("ssh %1 mkdir %2\n").arg(hostname).arg(remoteDir).toUtf8());
        ff.write(QString("tar -czf - %1 | ssh %2 'cd %3 ; tar -xzf -'\n").arg(localDir.absolutePath()).arg(hostname).arg(remoteDir).toUtf8());
        mNumberLines = mEngine->numberCommandLines();
        for (int i=0; i<mNumberLines; i++)
        {
            QString remoteCmd = QString("ssh %1 'cd %2%3 ;").arg(hostname).arg(remoteDir).arg(localDir.absolutePath());
            for (const std::string s : mEngine->commandLine(i))
            {
                remoteCmd += " " + QString::fromStdString(s);
            }
            remoteCmd +=  QString("'\n");
            ff.write(remoteCmd.toUtf8());
        }
        ff.write(QString("ssh %1 'cd %2%3 ; gzip -c %4' | gzip -dc > %5\n")
                 .arg(hostname)
                 .arg(remoteDir).arg(localDir.absolutePath())
                 .arg(resultFile).arg(localDir.absoluteFilePath(resultFile)).toUtf8());
        ff.write(QString("ssh %1 'rm -rf %2'\n").arg(hostname).arg(remoteDir).toUtf8());
        ff.close();
        ff.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);

        QStringList args;
        args << shellScriptName;

        if (!runProcess("bash",args))
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

            if (!runProcess(prog,args))
                return abortOnError();

            ++ mLineIndex;
        }

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);
    }

    bool SimulationProcess::runProcess(const QString& prog, const QStringList& args)
    {
        QTextStream xout(stdout, QIODevice::WriteOnly);
        xout << "executing <" << prog << ">";
        for (const QString& arg : args) xout << " [" << arg << "]";
        xout << "\n";
        QProcess* process = new QProcess;
        process->setWorkingDirectory(QString::fromStdString(mEngine->directory()));
        process->start(prog, args);

        if (!process->waitForStarted())
            return false;

        bool success = process->waitForFinished(-1);

        xout << "process stdout........\n";
        xout << process->readAllStandardOutput();
        xout << "\n--------------------\n";
        xout << "process stderr......\n";
        xout << process->readAllStandardError();
        xout << "\n====================\n";

        if (!success) return false;

        if (process->exitStatus() != QProcess::NormalExit || process->exitCode() != 0)
            return false;

        return true;
    }

}

#include "netlist_simulator_controller/simulation_process.h"

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/saleae_directory.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QThread>
#include <QUuid>
#include <vector>

namespace hal
{

    QString SimulationProcessLog::sLogFilename = "engine_log.html";

    SimulationProcessLog::SimulationProcessLog(const QString& filename) : mStream(nullptr)
    {
        mFile.setFileName(filename);
        if (mFile.open(QIODevice::WriteOnly))
            mStream = new QTextStream(&mFile);
    }

    SimulationProcessLog::SimulationProcessLog(bool toStdErr)
    {
        mStream = new QTextStream(toStdErr ? stderr : stdout, QIODevice::WriteOnly);
    }

    SimulationProcessLog::~SimulationProcessLog()
    {
        if (mStream)
        {
            mStream->flush();
            delete mStream;
        }
        if (mFile.isOpen())
            mFile.close();
    }

    SimulationProcessLog* SimulationProcessLog::logFactory(const std::string& workdirName)
    {
        QDir workDir(QString::fromStdString(workdirName));
        QString filename(workDir.absoluteFilePath(sLogFilename));

        SimulationProcessLog* retval = new SimulationProcessLog(filename);
        if (retval->good())
            return retval;
        delete retval;
        return new SimulationProcessLog(true);
    }

    SimulationProcess::SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine)
        : mEngine(engine), mLineIndex(0), mNumberLines(0), mSaleaeDirectoryFilename(controller->get_saleae_directory_filename()), mProcessLog(nullptr)
    {
        connect(this, &SimulationProcess::processFinished, controller, &NetlistSimulatorController::handleRunFinished);
    }

    void SimulationProcess::abortOnError()
    {
        mEngine->failed();
        Q_EMIT processFinished(false);
    }

    void SimulationProcess::openHtmlLog()
    {
    }

    void SimulationProcess::run()
    {
        mProcessLog = SimulationProcessLog::logFactory(mEngine->get_working_directory());
        if (mEngine->get_engine_property("ssh_server").empty())
            runLocal();
        else
            runRemote();
        if (mProcessLog)
            delete mProcessLog;
    }

    void SimulationProcess::runRemote()
    {
        SaleaeDirectory sd(mSaleaeDirectoryFilename);
        QFileInfo finfo(QString::fromStdString(mSaleaeDirectoryFilename));
        QString saleaeFilesToCopy = finfo.fileName();
        for (int inx = 0; inx < sd.get_next_available_index(); inx++)
        {
            saleaeFilesToCopy += QString(" digital_%1.bin").arg(inx);
        }

        QDir localDir(QString::fromStdString(mEngine->get_working_directory()));
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
        ff.write("set -x\n");    // echo commands
        ff.write("HOSTNAME=" + hostname.toUtf8() + "\n");
        ff.write("LOCALDIR=" + localDir.absolutePath().toUtf8() + "\n");
        ff.write(QString("if ssh ${HOSTNAME} mkdir -p /tmp/%1; then\n").arg(remoteDir).toUtf8());
        ff.write(QString("   REMOTEDIR=/tmp/%1\n").arg(remoteDir).toUtf8());
        ff.write("else\n");
        ff.write(QString("   ssh ${HOSTNAME} mkdir %1\n").arg(remoteDir).toUtf8());
        ff.write(QString("   REMOTEDIR=%1\n").arg(remoteDir).toUtf8());
        ff.write("fi\n");
        ff.write("set -e\n");    // bailout on error
        ff.write("tar -czf - ${LOCALDIR} | ssh ${HOSTNAME} \"cd ${REMOTEDIR} ; tar -xzf -\"\n");
        ff.write("ssh ${HOSTNAME} \"mkdir -p ${REMOTEDIR}${LOCALDIR}/saleae\"\n");
        ff.write(QString("cd %1; tar -czf - %2 | ssh ${HOSTNAME} \"cd ${REMOTEDIR}${LOCALDIR}/saleae ; tar -xzf -\"\n").arg(finfo.absolutePath()).arg(saleaeFilesToCopy).toUtf8());
        mNumberLines = mEngine->numberCommandLines();
        for (int i = 0; i < mNumberLines; i++)
        {
            QString remoteCmd("ssh ${HOSTNAME} \"cd ${REMOTEDIR}${LOCALDIR} ;");
            for (const std::string s : mEngine->commandLine(i))
            {
                remoteCmd += " " + QString::fromStdString(s);
            }
            remoteCmd += QString("\"\n");
            ff.write(remoteCmd.toUtf8());
        }
        ff.write(QString("ssh %1 \"cd ${REMOTEDIR}%2 ; gzip -c %3\" | gzip -dc > %4\n").arg(hostname).arg(localDir.absolutePath()).arg(resultFile).arg(localDir.absoluteFilePath(resultFile)).toUtf8());
        //        ff.write(QString("ssh %1 \"rm -rf ${REMOTEDIR}\"\n").arg(hostname).toUtf8());
        ff.close();
        ff.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

        QStringList args;
        args << shellScriptName;

        if (!runProcess("bash", args))
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
            for (const std::string& s : mEngine->commandLine(mLineIndex))
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

            if (!runProcess(prog, args))
                return abortOnError();

            ++mLineIndex;
        }

        if (!mEngine->finalize())
            return abortOnError();

        Q_EMIT processFinished(true);
    }

    QByteArray SimulationProcess::toHtml(const QByteArray& txt)
    {
        QByteArray retval;
        for (char cc : txt)
        {
            switch (cc)
            {
                case '<':
                    retval += "&lt;";
                    break;
                case '>':
                    retval += "&gt;";
                    break;
                case '&':
                    retval += "&amp;";
                    break;
                default:
                    retval += cc;
                    break;
            }
        }
        return retval;
    }

    bool SimulationProcess::runProcess(const QString& prog, const QStringList& args)
    {
        (*mProcessLog->log()) << "<html><body bgcolor=\"#000000\">\n";

        (*mProcessLog->log()) << "<h1><font color=\"#ffffff\">" << prog;
        for (const QString& arg : args)
            (*mProcessLog->log()) << " " << arg;
        (*mProcessLog->log()) << "</font></h1>\n";
        QProcess* process = new QProcess;
        process->setWorkingDirectory(QString::fromStdString(mEngine->get_working_directory()));
        process->start(prog, args);

        if (!process->waitForStarted())
        {
            qDebug() << "process wont start" << prog;
            return false;
        }

        bool success = process->waitForFinished(-1);

        for (const QByteArray& line : process->readAllStandardError().split('\n'))
        {
            if (line.startsWith("%Error"))
                (*mProcessLog->log()) << "<p><font color=\"#ff4040\">";
            else if (line.startsWith("%Warning"))
                (*mProcessLog->log()) << "<p><font color=\"#ffe050\">";
            else
                (*mProcessLog->log()) << "<p><font color=\"#ffffff\">";
            (*mProcessLog->log()) << toHtml(line) << "</font></p>\n";
        }

        for (const QByteArray& line : process->readAllStandardOutput().split('\n'))
        {
            (*mProcessLog->log()) << "<p><font color=\"#e0e0e0\">" << toHtml(line) << "</font></p>\n";
        }

        (*mProcessLog->log()) << "</body></html>\n";
        if (!success)
            return false;

        if (process->exitStatus() != QProcess::NormalExit || process->exitCode() != 0)
            return false;

        return true;
    }

}    // namespace hal

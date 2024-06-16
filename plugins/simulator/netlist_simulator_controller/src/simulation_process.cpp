#include "hal_core/utilities/log.h"
#include "netlist_simulator_controller/simulation_process.h"

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/saleae_directory.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QCoreApplication>
#include <QThread>
#include <QUuid>
#include <vector>

namespace hal
{

    QString SimulationProcessLog::sLogFilename = "engine_log.html";

    SimulationLogReceiver::SimulationLogReceiver(QObject* parent)
        : QObject(parent)
    {;}

    SimulationProcessLog::SimulationProcessLog(const QString& workdir, QObject* parent)
        : QObject(parent), mStream(nullptr)
    {
        QString filename(QDir(workdir).absoluteFilePath(sLogFilename));
        mFile.setFileName(filename);
        if (mFile.open(QIODevice::WriteOnly))
            mStream = new QTextStream(&mFile);
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

    void SimulationProcessLog::setLogReceiver(SimulationLogReceiver *logReceiver)
    {
        if (!logReceiver) return;
        connect(this,&SimulationProcessLog::processOutput,logReceiver,&SimulationLogReceiver::handleLog,Qt::QueuedConnection);
    }

    void SimulationProcessLog::flush()
    {
        if (mStream)
            mStream->flush();
    }

    void SimulationProcessLog::operator<< (const QString& txt)
    {
        if (mStream)
            (*mStream) << txt;
        Q_EMIT processOutput(txt);
    }

    SimulationProcess::SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine)
        : mEngine(engine), mLineIndex(0), mNumberLines(0), mSaleaeDirectoryFilename(controller->get_saleae_directory_filename()),
          mProcessLog(nullptr), mProcess(nullptr)
    {
        connect(this, &SimulationProcess::processFinished, controller, &NetlistSimulatorController::handleRunFinished);
        mProcessLog = new SimulationProcessLog(QString::fromStdString(mEngine->get_working_directory()));
    }

    SimulationProcess::~SimulationProcess()
    {
        if (mProcessLog) mProcessLog->deleteLater();
        if (mProcess)    mProcess->deleteLater();
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
        if (mEngine->get_engine_property("ssh_server").empty())
            runLocal();
        else
            runRemote();
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
            log_warning("simulation_plugin", "cannot open remote script '{}' for writing", shellScriptName.toStdString());
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

            if (mProcess)
            {
                delete mProcess;
                mProcess = nullptr;
            }
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
        QString logCommand = "<html><body bgcolor=\"#000000\">\n<h1><font color=\"#ffffff\">" + prog;
        for (const QString& arg : args)
            logCommand +=  " " + arg;
        logCommand += "</font></h1>\n";
        mProcess = new QProcess;
        mProcess->setWorkingDirectory(QString::fromStdString(mEngine->get_working_directory()));
        connect(mProcess,&QProcess::readyReadStandardError,this,&SimulationProcess::handleReadyReadStandardError);
        connect(mProcess,&QProcess::readyReadStandardOutput,this,&SimulationProcess::handleReadyReadStandardOutput);
        connect(mProcess,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this](int exitCode, QProcess::ExitStatus exitStatus){ this->handleFinished(exitCode,exitStatus); });

        (*mProcessLog) << logCommand;

        mProcess->start(prog, args);

        if (!mProcess->waitForStarted())
        {
            log_warning("simulation_plugin", "Cannot start process '{}' with args '{}'",
                        prog.toStdString(), args.join(' ').toStdString());
            if (prog == "verilator")
                log_warning("simulation_plugin", "You might want to check whether verilator has been installed on system");
            return false;
        }

        if (qApp)
        {
            if (exec())  // event loop ended with non-zero value
                return false;
        }
        else
        {
            log_warning("simulation_plugin", "No QApplication running, event loop not entered, will poll for process to finish.");
            if (!mProcess->waitForFinished()) return false;
            handleReadyReadStandardError();
            handleReadyReadStandardOutput();
        }

        return true;
    }

    void SimulationProcess::handleReadyReadStandardError()
    {
        QString errTxt;
        for (const QByteArray& line : mProcess->readAllStandardError().split('\n'))
        {
            if (line.startsWith("%Error"))
                errTxt += "<p><font color=\"#ff4040\">";
            else if (line.startsWith("%Warning"))
                errTxt += "<p><font color=\"#ffe050\">";
            else
                errTxt += "<p><font color=\"#ffffff\">";

            errTxt += QString::fromUtf8(toHtml(line)) + "</font></p>\n";
        }

        (*mProcessLog) << errTxt;
    }

    void SimulationProcess::handleReadyReadStandardOutput()
    {
        QString outTxt;

        for (const QByteArray& line : mProcess->readAllStandardOutput().split('\n'))
        {
             outTxt += "<p><font color=\"#e0e0e0\">" + QString::fromUtf8(toHtml(line)) + "</font></p>\n";
        }

        (*mProcessLog) << outTxt;
    }

    void SimulationProcess::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        QString endTxt = QString("<p>exit code %1</p></body></html>\n").arg(exitCode);
        (*mProcessLog) << endTxt;
        mProcessLog->flush();

        if (exitCode != 0) exit(exitCode);

        if (exitStatus != QProcess::NormalExit) exit(-99);

        exit(0);
    }
}    // namespace hal

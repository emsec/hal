#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QProcess>
#include <QTextStream>
#include <QFile>
#include <QThread>

namespace hal {

    class NetlistSimulatorController;

    class SimulationProcessLog
    {
        QFile mFile;
        QTextStream *mStream;
        SimulationProcessLog(const QString& filename);
        SimulationProcessLog(bool toStdErr);
    public:
        ~SimulationProcessLog();
        bool good() const { return mFile.isOpen(); }
        QTextStream* log() const { return mStream; }
        static SimulationProcessLog* logFactory(const std::string& workdirName);
        static QString sLogFilename;
    };

    class SimulationProcess : public QThread {
        Q_OBJECT

        SimulationEngineScripted* mEngine;

        int mLineIndex;
        int mNumberLines;
        std::string mSaleaeDirectoryFilename;
        SimulationProcessLog* mProcessLog;

        void abortOnError();
        void openHtmlLog();
        bool runProcess(const QString& prog, const QStringList& args);

    Q_SIGNALS:
        void processFinished(bool success);

    public:
        SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine);

        void run();
    private:
        QByteArray toHtml(const QByteArray& txt);
        void runLocal();

        // server execution
        void runRemote();

    };
}

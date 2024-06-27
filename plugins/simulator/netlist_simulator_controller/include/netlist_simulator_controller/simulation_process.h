// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QProcess>
#include <QTextStream>
#include <QFile>
#include <QThread>

namespace hal {

    class NetlistSimulatorController;

    class SimulationLogReceiver : public QObject
    {
        Q_OBJECT
    public Q_SLOTS:
        virtual void handleLog(const QString& txt) {;}
    public:
        SimulationLogReceiver(QObject* parent = nullptr);
        virtual ~SimulationLogReceiver() {;}
    };


    class SimulationProcessLog : public QObject
    {
        Q_OBJECT

        QFile mFile;
        QTextStream *mStream;
    Q_SIGNALS:
        void processOutput(QString txt);

    public:
        SimulationProcessLog(const QString& workdir, QObject* parent = nullptr);
        virtual ~SimulationProcessLog();
        bool good() const { return mFile.isOpen(); }
        void setLogReceiver(SimulationLogReceiver* logReceiver);
        void operator<< (const QString& txt);
        void flush();
        static QString sLogFilename;
    };

    class SimulationProcess : public QThread {
        Q_OBJECT

        SimulationEngineScripted* mEngine;

        int mLineIndex;
        int mNumberLines;
        std::string mSaleaeDirectoryFilename;
        SimulationProcessLog* mProcessLog;

        QProcess* mProcess;
        void abortOnError();
        void openHtmlLog();
        bool runProcess(const QString& prog, const QStringList& args);

    Q_SIGNALS:
        void processFinished(bool success);

    private Q_SLOTS:
        void handleReadyReadStandardError();
        void handleReadyReadStandardOutput();
        void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);

    public:
        SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine);
        virtual ~SimulationProcess();
        SimulationProcessLog* log() const { return mProcessLog; }
        void run();
    private:
        QByteArray toHtml(const QByteArray& txt);
        void runLocal();

        // server execution
        void runRemote();

    };
}

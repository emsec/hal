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

#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QString>

class QLocalServer;
class QLocalSocket;

namespace hal
{
    class PythonContext;

    /**
     * @ingroup gui
     * @brief Lets another process run Python inside the GUI.
     *
     * Listens on a local socket, enabled with `--remote <path>`. A client writes one JSON object per line and reads
     * JSON objects back until one carries `done`:
     *
     * - `{"code": "..."}` runs the code the way the Python editor would, with `netlist` and `gui` predefined.
     *   `{"file": "..."}` runs a script file. While it runs, `{"stdout": "..."}` and `{"stderr": "..."}` objects
     *   carry its output, and `{"done": true, "exit_code": N}` ends the reply: 0 when the code ran to its end,
     *   the number given to `sys.exit`, 1 on an unhandled exception.
     * - `{"abort": true}` interrupts whatever is running and is answered with `{"done": true, "exit_code": 1}` on
     *   the request it interrupted.
     *
     * Requests run one at a time, in the order they arrive, and wait for anything the console or editor is running.
     * The socket is only accessible to the user running HAL.
     */
    class RemoteControl : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Construct a remote control that hands its requests to the given Python context.
         *
         * @param[in] context - The Python context of the GUI.
         * @param[in] parent - The parent object.
         */
        explicit RemoteControl(PythonContext* context, QObject* parent = nullptr);

        /**
         * Start listening on the given socket path, replacing a stale socket file left by an earlier process.
         *
         * @param[in] socketPath - The path of the local socket.
         * @returns `true` on success, `false` if the socket could not be opened.
         */
        bool listen(const QString& socketPath);

        /**
         * The path the server listens on.
         *
         * @returns The socket path, empty when not listening.
         */
        QString socketPath() const;

    private Q_SLOTS:
        void handleNewConnection();
        void handleReadyRead();
        void handleDisconnected();
        void handleOutput(const QString& text, bool isError);
        void handleFinished(int exitCode, const QString& errorMessage);

    private:
        struct Request
        {
            QPointer<QLocalSocket> socket;
            QString code;
        };

        void startNext();
        void reply(QLocalSocket* socket, const QJsonObject& object);

        PythonContext* mContext;
        QLocalServer* mServer;
        QQueue<Request> mQueue;
        Request mActive;
        bool mRunning = false;
    };
}    // namespace hal

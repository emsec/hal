#include "gui/remote_control/remote_control.h"

#include "gui/python/python_context.h"
#include "hal_core/utilities/log.h"

#include <QFile>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>

namespace hal
{
    RemoteControl::RemoteControl(PythonContext* context, QObject* parent) : QObject(parent), mContext(context), mServer(new QLocalServer(this))
    {
        mServer->setSocketOptions(QLocalServer::UserAccessOption);
        connect(mServer, &QLocalServer::newConnection, this, &RemoteControl::handleNewConnection);
        connect(mContext, &PythonContext::scriptOutput, this, &RemoteControl::handleOutput);
        connect(mContext, &PythonContext::scriptFinished, this, &RemoteControl::handleFinished);
    }

    bool RemoteControl::listen(const QString& socketPath)
    {
        // a socket file left behind by a process that no longer exists would block the address
        QLocalServer::removeServer(socketPath);
        if (!mServer->listen(socketPath))
        {
            log_error("gui", "cannot listen for remote control on '{}': {}", socketPath.toStdString(), mServer->errorString().toStdString());
            return false;
        }
        log_info("gui", "remote control listening on '{}'", socketPath.toStdString());
        return true;
    }

    QString RemoteControl::socketPath() const
    {
        return mServer->isListening() ? mServer->fullServerName() : QString();
    }

    void RemoteControl::handleNewConnection()
    {
        while (QLocalSocket* socket = mServer->nextPendingConnection())
        {
            connect(socket, &QLocalSocket::readyRead, this, &RemoteControl::handleReadyRead);
            connect(socket, &QLocalSocket::disconnected, this, &RemoteControl::handleDisconnected);
        }
    }

    void RemoteControl::handleReadyRead()
    {
        auto* socket = qobject_cast<QLocalSocket*>(sender());
        if (!socket)
        {
            return;
        }
        while (socket->canReadLine())
        {
            const QByteArray line = socket->readLine().trimmed();
            if (line.isEmpty())
            {
                continue;
            }
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject())
            {
                reply(socket, {{"error", "expected one JSON object per line: " + parseError.errorString()}, {"done", true}, {"exit_code", 1}});
                continue;
            }
            const QJsonObject request = document.object();

            if (request.value("abort").toBool())
            {
                if (mRunning)
                {
                    mContext->abortThread();
                }
                reply(socket, {{"done", true}, {"exit_code", 0}});
                continue;
            }

            QString code;
            if (request.contains("code"))
            {
                code = request.value("code").toString();
            }
            else if (request.contains("file"))
            {
                QFile file(request.value("file").toString());
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    reply(socket, {{"error", "cannot read '" + file.fileName() + "'"}, {"done", true}, {"exit_code", 1}});
                    continue;
                }
                code = QString::fromUtf8(file.readAll());
            }
            else
            {
                reply(socket, {{"error", "a request needs 'code', 'file' or 'abort'"}, {"done", true}, {"exit_code", 1}});
                continue;
            }

            mQueue.enqueue({socket, code});
            startNext();
        }
    }

    void RemoteControl::handleDisconnected()
    {
        auto* socket = qobject_cast<QLocalSocket*>(sender());
        if (!socket)
        {
            return;
        }
        // a request of a client that left still runs to its end, its output just has nowhere to go
        socket->deleteLater();
    }

    void RemoteControl::startNext()
    {
        if (mRunning || mQueue.isEmpty() || mContext->isThreadRunning())
        {
            // whatever the console or editor is running finishes first; scriptFinished brings us back here
            return;
        }
        mActive  = mQueue.dequeue();
        mRunning = true;
        mContext->runScript(mActive.code);
    }

    void RemoteControl::handleOutput(const QString& text, bool isError)
    {
        if (mRunning && mActive.socket)
        {
            reply(mActive.socket, {{isError ? "stderr" : "stdout", text}});
        }
    }

    void RemoteControl::handleFinished(int exitCode, const QString& errorMessage)
    {
        Q_UNUSED(errorMessage);    // already delivered through handleOutput as stderr
        if (mRunning)
        {
            mRunning = false;
            if (mActive.socket)
            {
                reply(mActive.socket, {{"done", true}, {"exit_code", exitCode}});
            }
        }
        startNext();
    }

    void RemoteControl::reply(QLocalSocket* socket, const QJsonObject& object)
    {
        if (!socket || socket->state() != QLocalSocket::ConnectedState)
        {
            return;
        }
        socket->write(QJsonDocument(object).toJson(QJsonDocument::Compact) + "\n");
        socket->flush();
    }
}    // namespace hal

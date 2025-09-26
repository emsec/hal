#include "cross_probing/cross_probing_server.h"
#include <QXmlStreamReader>
#include <QLocalSocket>
#include <QMetaEnum>
#include "gui/user_action/user_action_manager.h"
#include "gui/user_action/user_action_compound.h"
#include "cross_probing/cross_probing_server.h"

namespace hal {

    CrossProbingServer::CrossProbingServer(QObject* parent)
    {
        mServer = new QLocalServer(this);
        connect(mServer, &QLocalServer::newConnection, this, &CrossProbingServer::receiveCommand, Qt::QueuedConnection);
        mServer->listen("hal_action_pipe");
    }

    CrossProbingServer::~CrossProbingServer()
    {
        mServer->close();
        mServer->deleteLater();
    }

    void CrossProbingServer::receiveCommand()
    {
        mReturnMessage.clear();

        QLocalSocket *clientConnection = mServer->nextPendingConnection();
        connect(clientConnection, &QLocalSocket::disconnected,
                clientConnection, &QLocalSocket::deleteLater);

        clientConnection->waitForReadyRead();
        QByteArray inMsg = clientConnection->readAll();

        handleCommand(inMsg);

        clientConnection->write(mReturnMessage.toUtf8());
        clientConnection->flush();
        clientConnection->disconnectFromServer();
    }

    bool CrossProbingServer::handleCommand(QByteArray cmd)
    {
        QXmlStreamReader xmlIn(cmd);

        UserActionCompound* compoundAction = nullptr;
        while (!xmlIn.atEnd())
        {
            if (xmlIn.readNext())
            {
                if (xmlIn.isStartElement())
                {
                    if (xmlIn.name() == "compound")
                    {
                        compoundAction = new UserActionCompound;
                    }
                    else
                    {
                        UserAction* act = UserActionManager::instance()->getParsedAction(xmlIn);
                        if (!act)
                        {
                            if (compoundAction) delete compoundAction;
                            mReturnMessage ="Could not create action for " + xmlIn.name();
                            return false;
                        }
                        if (compoundAction)
                        {
                            compoundAction->addAction(act);
                        }
                        else
                        {
                            if (act->exec())
                            {
                                mReturnMessage = "Successfully executed action " + act->tagname();
                            }
                            else
                            {
                                mReturnMessage = "Error executing action" + act->tagname();
                                return false;
                            }
                        }
                    }
                }
            }
        }
        if (compoundAction)
        {
            QMetaEnum meta = QMetaEnum::fromType<UserActionObjectType::ObjectType>(); // type enum to string
            compoundAction->setUseCreatedObject();
            if (compoundAction->exec())
                mReturnMessage =
                                 QString("Successfully executed compund action with %1 object ID=%2")
                                     .arg(meta.key(compoundAction->object().type()))
                                     .arg(compoundAction->object().id());
            else
            {
                mReturnMessage = "Error executing compound action";
                return false;
            }
        }

        if (xmlIn.hasError())
        {
            mReturnMessage = "XML parser error";
            return false;
        }

        return true;
    }
}

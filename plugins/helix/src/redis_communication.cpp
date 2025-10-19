#include <iostream>

#include <QCoreApplication>
#include <QTimer>
#include <QVariant>

#include "helix/hiredis_qt.h"

#include "helix/redis_communication.h"
#include "gui/gui_globals.h"

namespace hal {

    RedisCommunication::RedisCommunication(QObject* parent)
        : QObject(parent), m_ctxSubs(nullptr), m_ctxPubl(nullptr), m_netlist(nullptr)
    {
        m_subscriber = new RedisQtAdapter(this); // child object should get deleted automatically in destructor
        m_publisher = new RedisQtAdapter(this);
    }

    RedisCommunication::~RedisCommunication()
    {
        if (m_ctxSubs) delete m_ctxSubs;
        if (m_ctxPubl) delete m_ctxPubl;
    }

    void getCallback(redisAsyncContext *, void * r, void * privdata) {
        RedisCommunication* rcomm = static_cast<RedisCommunication *>(privdata);
        if (rcomm == nullptr)
        {
            std::cerr << "Internal hiredis error, handler class pointer not returned" << std::endl;
            return;
        }
        redisReply * reply = static_cast<redisReply *>(r);
        if (reply == nullptr)
            rcomm->handleNoReply();
        else
            rcomm->handleIncomingMessage(reply);
    }

    void RedisCommunication::rememberNetlist(const Netlist* nl)
    {
        m_netlist = nl;
    }

    void RedisCommunication::handleNoReply()
    {
        Q_EMIT errorMessage("No reply from redis in context, please check whether server is running");
    }


    void RedisCommunication::handleIncomingMessage(redisReply* reply)
    {
        if (m_netlist && gNetlist != m_netlist)
        {
            Q_EMIT errorMessage("Netlist has changed while helix running");
            return;
        }

        switch (reply->type)
        {
        case REDIS_REPLY_ERROR:
            Q_EMIT errorMessage("Redis returned error <" + QString(reply->str) + ">");
            break;
        case REDIS_REPLY_STATUS:
            Q_EMIT errorMessage("Redis returned status <" + QString(reply->str) + "> instead of message");
            break;
        case REDIS_REPLY_ARRAY:
        {
            QList<QVariant> elements;
            for (int i=0; i<reply->elements; i++)
            {
                switch (reply->element[i]->type) {
                case REDIS_REPLY_INTEGER:
                    elements.append(reply->element[i]->integer);
                    break;
                case REDIS_REPLY_STRING:
                    elements.append(QString(reply->element[i]->str));
                    break;
                default:
                    Q_EMIT errorMessage(QString("Error parsing redis array, element %d has type %d)").arg(i).arg(reply->element[i]->type));
                    break;
                }
            }

            if (elements.size() == 3 && elements.at(0).toString() == "message")
                Q_EMIT messageReceived(elements.at(2).toString(), elements.at(1).toString());
            break;
        }
        default:
            Q_EMIT errorMessage("Redis returned unknown reply type " + QString::number(reply->type));
            break;
        }
    }

    void RedisCommunication::publish(std::string channel, std::string msg)
    {
        std::string output = "PUBLISH " + channel + " " + msg;
        redisAsyncCommand(m_ctxPubl, nullptr, nullptr, output.c_str());
    }


    bool RedisCommunication::subscribeToChannels(std::string host, int port, const std::vector<std::string>& channels) {

        m_ctxSubs = redisAsyncConnect(host.c_str(), port);
        m_ctxPubl = redisAsyncConnect(host.c_str(), port);

        if (m_ctxSubs->err || m_ctxPubl->err) {
            redisAsyncFree(m_ctxSubs);
            redisAsyncFree(m_ctxPubl);
            Q_EMIT errorMessage("Redis connection error: <" + QString(m_ctxSubs->errstr) + "> <" + QString(m_ctxPubl->errstr) + ">");
            return false;
        }

        m_subscriber->setContext(m_ctxSubs);
        RedisQtAddRead(m_subscriber);

        m_publisher->setContext(m_ctxPubl);
        RedisQtAddWrite(m_publisher);

        for (std::string channel : channels)
        {
            redisAsyncCommand(m_ctxSubs, getCallback, this, ("SUBSCRIBE "+channel).c_str());
        }
        return true;
    }
}

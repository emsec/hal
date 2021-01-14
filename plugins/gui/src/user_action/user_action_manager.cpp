#include "gui/user_action/user_action_manager.h"
#include <QFile>
#include <QMetaEnum>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include "gui/user_action/user_action.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/user_action/action_open_netlist_file.h"
#include "gui/user_action/action_unfold_module.h"

namespace hal
{
    UserActionManager* UserActionManager::inst = nullptr;

    UserActionManager::UserActionManager(QObject *parent)
        : QObject(parent), mStartRecording(-1),
          mWaitCount(0)
    {
        mElapsedTime.start();
    }

    void UserActionManager::addExecutedAction(UserAction* act)
    {
        mActionHistory.append(act);
    }

    void UserActionManager::setStartRecording()
    {
        mStartRecording = mActionHistory.size();
    }

    void UserActionManager::setStopRecording(const QString& macroFilename)
    {
        int n = mActionHistory.size();
        if (n>mStartRecording && !macroFilename.isEmpty())
        {
            QFile of(macroFilename);
            if (of.open(QIODevice::WriteOnly))
            {
                QXmlStreamWriter xmlOut(&of);
                xmlOut.setAutoFormatting(true);
                xmlOut.writeStartDocument();
                xmlOut.writeStartElement("actions");

                for (int i=mStartRecording; i<n; i++)
                {
                    const UserAction* act = mActionHistory.at(i);
                    xmlOut.writeStartElement(act->tagname());
                    // TODO : enable / disable timestamp and crypto hash by user option ?
                    xmlOut.writeAttribute("ts",QString::number(act->timeStamp()));
                    xmlOut.writeAttribute("md5",act->cryptographicHash(i-mStartRecording));
                    act->object().writeToXml(xmlOut);
                    act->writeToXml(xmlOut);
                    xmlOut.writeEndElement();
                }
                xmlOut.writeEndElement();
                xmlOut.writeEndDocument();
            }
        }
        mStartRecording = -1;
    }

    void UserActionManager::playMacro(const QString& macroFilename)
    {
        QFile ff(macroFilename);
        bool parseActions = false;
        if (!ff.open(QIODevice::ReadOnly)) return;
        QXmlStreamReader xmlIn(&ff);
        mStartRecording = mActionHistory.size();
        while (!xmlIn.atEnd())
        {
            if (xmlIn.readNext())
            {
                if (xmlIn.isStartElement())
                {
                    if (xmlIn.name() == "actions")
                        parseActions = true;
                    else if (parseActions)
                    {
                        UserAction* act = getParsedAction(xmlIn);
                        if (act) mActionHistory.append(act);
                    }
                }
                else if (xmlIn.isEndElement())
                {
                    if (xmlIn.name() == "actions")
                        parseActions = false;
                }
             }
        }
        if (xmlIn.hasError())
        {
            // TODO : error message
            return;
        }

        int endMacro = mActionHistory.size();
        for (int i=mStartRecording; i<endMacro; i++)
        {
            UserAction* act = mActionHistory.at(i);
            act->exec();
            if (act->isWaitForReady()) mWaitCount=100;
            while (mWaitCount > 0)
            {
                --mWaitCount;
                qDebug() << "wait" << mWaitCount;
                QCoreApplication::processEvents();
                QThread::msleep(10);
            }
        }
        mStartRecording = -1;
    }

    UserAction* UserActionManager::getParsedAction(QXmlStreamReader& xmlIn) const
    {
        QString actionTagName = xmlIn.name().toString();

        UserActionFactory* fac = mActionFactory.value(actionTagName);
        if (!fac)
        {
            qDebug() << "cannot parse user action" << actionTagName;
            return nullptr;
        }
        UserAction* retval = fac->newAction();
        if (retval)
        {
            UserActionObject actObj;
            actObj.readFromXml(xmlIn);
            retval->setObject(actObj);
            retval->readFromXml(xmlIn);
        }

        return retval;
    }

    bool UserActionManager::hasRecorded() const
    {
        return isRecording() && mActionHistory.size() > mStartRecording;
    }

    bool UserActionManager::isRecording() const
    {
        return mStartRecording >= 0;
    }

    void UserActionManager::registerFactory(UserActionFactory* fac)
    {
        mActionFactory.insert(fac->tagname(),fac);
    }

    UserActionManager* UserActionManager::instance()
    {
        if (!inst) inst = new UserActionManager;
        return inst;
    }
}

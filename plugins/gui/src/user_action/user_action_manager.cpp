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
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "hal_core/utilities/log.h"
#include <QTextCursor>
#include <QMessageBox>
#include <QApplication>

namespace hal
{
    UserActionManager* UserActionManager::inst = nullptr;

    UserActionManager::UserActionManager(QObject *parent)
        : QObject(parent), mStartRecording(-1),
          mRecordHashAttribute(true),
          mDumpAction(nullptr),
          mThreadedAction(nullptr)
    {
        mElapsedTime.start();
        mSettingDumpAction = new SettingsItemCheckbox(
            "UserAction Debug",
            "debug/user_action",
            false,
            "eXpert Settings:Debug",
            "Specifies whether hal opens an extra window to list all executed instances of UserAction"
        );
        connect(mSettingDumpAction,&SettingsItemCheckbox::boolChanged,this,&UserActionManager::handleSettingDumpActionChanged);
        connect(this,&UserActionManager::triggerExecute,this,&UserActionManager::handleTriggerExecute,Qt::BlockingQueuedConnection);
    }

    void UserActionManager::executeActionBlockThread(UserAction *act)
    {
        if (!act) return;
        mMutex.lock();
        mThreadedAction = act;
        Q_EMIT triggerExecute();
        mMutex.unlock();
    }

    void UserActionManager::handleTriggerExecute()
    {
        mThreadedAction->exec();
    }

    void UserActionManager::handleSettingDumpActionChanged(bool wantDump)
    {
        if (!wantDump && mDumpAction)
        {
            mDumpAction->deleteLater();
            mDumpAction = nullptr;
        }
    }

    void UserActionManager::addExecutedAction(UserAction* act)
    {
        mActionHistory.append(act);

        if (mSettingDumpAction->value().toBool())
        {
            if (!mDumpAction)
            {
                mDumpAction = new QPlainTextEdit;
                mDumpAction->show();
            }

            mDumpAction->moveCursor (QTextCursor::End);
            mDumpAction->insertPlainText(act->debugDump());
            mDumpAction->moveCursor(QTextCursor::End);
        }
        testUndo();
    }

    void UserActionManager::setStartRecording()
    {
        mStartRecording = mActionHistory.size();
    }

    void UserActionManager::crashDump(int sig)
    {
        mStartRecording = 0;
        mRecordHashAttribute = false;
        setStopRecording(QString("hal_crashdump_signal%1.xml").arg(sig));
    }

    QMessageBox::StandardButton UserActionManager::setStopRecording(const QString& macroFilename)
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
                    if (mRecordHashAttribute)
                        xmlOut.writeAttribute("sha",act->cryptographicHash(i-mStartRecording));
                    if (act->compoundOrder() >= 0)
                        xmlOut.writeAttribute("compound",QString::number(act->compoundOrder()));
                    act->object().writeToXml(xmlOut);
//perhaps put this in all actions that need a parentobject? could be redundant though
//                    if(act->parentObject().type() != UserActionObjectType::None)
//                    {
//                        xmlOut.writeStartElement("parentObj");
//                        act->parentObject().writeToXml(xmlOut);
//                        xmlOut.writeEndElement();
//                    }
                    act->writeToXml(xmlOut);
                    xmlOut.writeEndElement();
                }
                xmlOut.writeEndElement();
                xmlOut.writeEndDocument();
            }
            else
            {
                log_warning("gui", "Failed to save macro to '{}.", macroFilename.toStdString());
                QMessageBox::StandardButton retval =
                        QMessageBox::warning(qApp->activeWindow(), "Save Macro Failed", "Cannot save macro to file\n<" + macroFilename + ">",
                                             QMessageBox::Retry | QMessageBox::Discard | QMessageBox::Cancel);
                if (retval == QMessageBox::Discard)
                    mStartRecording = -1;
                return retval;
            }
        }
        mStartRecording = -1;
        return QMessageBox::Ok;
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
            if (!act->exec())
            {
                log_warning("gui", "failed to execute user action {}", act->tagname().toStdString());
                break;
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
            QStringRef compound = xmlIn.attributes().value("compound");
            if (!compound.isNull() && !compound.isEmpty())
                retval->setCompoundOrder(compound.toInt());
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

    void UserActionManager::testUndo()
    {
        bool yesWeCan = true;
        if (mActionHistory.isEmpty())
            yesWeCan = false;
        else
        {
            auto it = mActionHistory.end() - 1;
            // compound can be reversed only if all actions have undo pointer
            while (it != mActionHistory.begin() &&
                   (*it)->undoAction() &&
                   (*it)->compoundOrder() > 0)
                --it;
            if (!(*it)->undoAction())
                yesWeCan = false;
        }
        Q_EMIT canUndoLastAction(yesWeCan);
    }

    void UserActionManager::undoLastAction()
    {
        if (mActionHistory.isEmpty()) return;
        QList<UserAction*> undoList;
        while (!mActionHistory.isEmpty())
        {
            UserAction* lastAction = mActionHistory.takeLast();
            if (!lastAction->undoAction())
                return;

            undoList.append(lastAction->undoAction());
            if (lastAction->compoundOrder() <= 0) break;
        }
        int n = mActionHistory.size();
        for (UserAction* act : undoList)
            act->exec();
        while (mActionHistory.size() > n)
            mActionHistory.takeLast();
        testUndo();
    }
}

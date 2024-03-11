#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/context_manager_widget/models/context_tree_model.h"

namespace hal
{
    ActionCreateObjectFactory::ActionCreateObjectFactory()
        : UserActionFactory("CreateObject") {;}

    ActionCreateObjectFactory* ActionCreateObjectFactory::sFactory = new ActionCreateObjectFactory;

    UserAction* ActionCreateObjectFactory::newAction() const
    {
        return new ActionCreateObject;
    }

    QString ActionCreateObject::tagname() const
    {
        return ActionCreateObjectFactory::sFactory->tagname();
    }

    ActionCreateObject::ActionCreateObject(UserActionObjectType::ObjectType type,
                                           const QString &objName)
      : mObjectName(objName), mParentId(0), mLinkedObjectId(0)
    {
        setObject(UserActionObject(0,type));
    }

    void ActionCreateObject::addToHash(QCryptographicHash &cryptoHash) const
    {
        cryptoHash.addData(mObjectName.toUtf8());
        cryptoHash.addData((char*)(&mParentId),sizeof(mParentId));
        cryptoHash.addData((char*)(&mLinkedObjectId),sizeof(mLinkedObjectId));
    }

    void ActionCreateObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        //todo: remove parentId, switch entirely to parentObject
        writeParentObjectToXml(xmlOut);
        xmlOut.writeTextElement("objectname", mObjectName);
        xmlOut.writeTextElement("parentid", QString::number(mParentId));
        xmlOut.writeTextElement("linkedid", QString::number(mLinkedObjectId));
    }

    void ActionCreateObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            //todo: emove parentId, switch entirely to parentObject
            readParentObjectFromXml(xmlIn);
            if (xmlIn.name() == "objectname")
                mObjectName = xmlIn.readElementText();
            if (xmlIn.name() == "parentid")
                mParentId = xmlIn.readElementText().toInt();
            if (xmlIn.name() == "linkedid")
                mLinkedObjectId = xmlIn.readElementText().toInt();
        }
    }

    bool ActionCreateObject::exec()
    {
        bool standardUndo = false;

        switch (mObject.type())
        {
            break;

        case UserActionObjectType::Module:
        {
            Module* parentModule = gNetlist->get_module_by_id(mParentId);
            if (parentModule)
            {
                u32 modId = mObject.id() ? mObject.id() : gNetlist->get_unique_module_id();
                Module* m = gNetlist->create_module(modId,
                                                mObjectName.toStdString(), parentModule);
                if (!m)
                {
                    log_warning("gui", "Failed to create module '{}' with ID={} under parent ID={}.",
                                mObjectName.toStdString(), modId, mParentId);
                    return false;
                }
                setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
                standardUndo = true;
            }
        }
            break;
        /*
        case UserActionObjectType::Gate:
        {
            Gate* g = gNetlist->create_gate(mGroupingName.toStdString());
            setObject(UserActionObject(g->get_id(),UserActionObjectType::Gate));
        }
            break;
            */
        case UserActionObjectType::Net:
        {
            Net* n = gNetlist->create_net(mObjectName.toStdString());
            setObject(UserActionObject(n->get_id(),UserActionObjectType::Net));
            standardUndo = true;
        }
            break;
        case UserActionObjectType::Grouping:
        {
            GroupingTableModel* grpModel = gContentManager->getGroupingManagerWidget()->getModel();
            if (!grpModel) return false;
            Grouping* grp = grpModel->addDefaultEntry();
            if (!mObjectName.isEmpty())
                grpModel->renameGrouping(grp->get_id(),mObjectName);
            setObject(UserActionObject(grp->get_id(),UserActionObjectType::Grouping));
            standardUndo = true;
        }
            break;
        case UserActionObjectType::ContextView:
        {
            QString contextName = mObjectName.isEmpty()
                    ? gGraphContextManager->nextDefaultName()
                    : mObjectName;
            GraphContext* ctxView = gGraphContextManager->createNewContext(contextName, mParentId);
            if (mLinkedObjectId > 0) ctxView->setExclusiveModuleId(mLinkedObjectId);
            if (mObject.id() > 0) gGraphContextManager->setContextId(ctxView,mObject.id());
            setObject(UserActionObject(ctxView->id(),UserActionObjectType::ContextView));
            standardUndo = true;
        }
            break;
        case UserActionObjectType::ContextDir:
        {
            QString directoryName = mObjectName.isEmpty()
                    ? gGraphContextManager->nextDefaultName()
                    : mObjectName;

            ContextDirectory* ctxDir = gGraphContextManager->createNewDirectory(directoryName, mParentId);
            if (mObject.id() > 0) ctxDir->setId(mObject.id());
            setObject(UserActionObject(ctxDir->id(),UserActionObjectType::ContextDir));
            standardUndo = true;
        }
            break;
        default:
            return false; // don't know how to create
        }
        if (standardUndo)
        {
            mUndoAction = new ActionDeleteObject;
            mUndoAction->setObject(mObject);
        }
        return UserAction::exec();
    }
}

#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"

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
      : mObjectName(objName), mParentId(0)
    {
        setObject(UserActionObject(0,type));
    }

    void ActionCreateObject::addToHash(QCryptographicHash &cryptoHash) const
    {
        cryptoHash.addData(mObjectName.toUtf8());
        cryptoHash.addData((char*)(&mParentId),sizeof(mParentId));
    }

    void ActionCreateObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        //todo: remove parentId, switch entirely to parentObject
        writeParentObjectToXml(xmlOut);
        xmlOut.writeTextElement("objectname", mObjectName);
        xmlOut.writeTextElement("parentid", QString::number(mParentId));
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
        }
    }

    bool ActionCreateObject::exec()
    {
        bool standardUndo = false;

        switch (mObject.type())
        {

        case UserActionObjectType::PinGroup:
        {
            Module* parentModule = gNetlist->get_module_by_id(mParentObject.id());
            if(parentModule)
            {
                auto res = parentModule->create_pin_group(mObjectName.toStdString());
                if(res.is_error())
                    return false;
                setObject(UserActionObject(res.get()->get_id(), UserActionObjectType::PinGroup));
                standardUndo = true;
            }
            else
                return false;
        }
            break;

        case UserActionObjectType::Module:
        {
            Module* parentModule = gNetlist->get_module_by_id(mParentId);
            if (parentModule)
            {
                Module* m = gNetlist->create_module(gNetlist->get_unique_module_id(),
                                                mObjectName.toStdString(), parentModule);
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
        case UserActionObjectType::Context:
        {
            QString contextName = mObjectName.isEmpty()
                    ? gGraphContextManager->nextDefaultName()
                    : mObjectName;
            GraphContext* ctx = gGraphContextManager->createNewContext(contextName);
            if (mObject.id() > 0) gGraphContextManager->setContextId(ctx,mObject.id());
            setObject(UserActionObject(ctx->id(),UserActionObjectType::Context));
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
            mUndoAction->setParentObject(mParentObject);
        }
        return UserAction::exec();
    }
}

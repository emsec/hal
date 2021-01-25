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
        xmlOut.writeTextElement("objectname", mObjectName);
        xmlOut.writeTextElement("parentid", QString::number(mParentId));
    }

    void ActionCreateObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "objectname")
                mObjectName = xmlIn.readElementText();
            if (xmlIn.name() == "parentid")
                mParentId = xmlIn.readElementText().toInt();
        }
    }

    void ActionCreateObject::exec()
    {
        switch (mObject.type())
        {

        case UserActionObjectType::Module:
        {
            Module* parentModule = gNetlist->get_module_by_id(mParentId);
            if (parentModule)
            {
                Module* m = gNetlist->create_module(gNetlist->get_unique_module_id(),
                                                mObjectName.toStdString(), parentModule);
                setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
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
        }
            break;
        case UserActionObjectType::Grouping:
        {
            GroupingTableModel* grpModel = gContentManager->getGroupingManagerWidget()->getModel();
            if (!grpModel) return;
            Grouping* grp = grpModel->addDefaultEntry();
            UserActionObject obj(grp->get_id(),UserActionObjectType::Grouping);
            if (!mObjectName.isEmpty())
                grpModel->renameGrouping(obj.id(),mObjectName);
            mUndoAction = new ActionDeleteObject;
            mUndoAction->setObject(obj);
            setObject(UserActionObject(grp->get_id(),UserActionObjectType::Grouping));
        }
            break;
        case UserActionObjectType::Context:
        {
            QString contextName = mObjectName.isEmpty()
                    ? gGraphContextManager->nextDefaultName()
                    : mObjectName;
            GraphContext* ctx = gGraphContextManager->createNewContext(contextName);
            setObject(UserActionObject(ctx->id(),UserActionObjectType::Context));
        }
            break;
        default:
            break; // don't know how to create
        }
        UserAction::exec();
    }
}

#include "gui/user_action/action_create_object.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"
#include "gui/grouping/grouping_manager_widget.h"

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

    ActionCreateObject::ActionCreateObject(const QString &objName)
      : mObjectName(objName)
    {;}

    void ActionCreateObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("groupingname", mObjectName);
    }

    void ActionCreateObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "groupingname")
                mObjectName = xmlIn.readElementText();
        }
    }

    void ActionCreateObject::exec()
    {
        switch (mObject.type())
        {
        /*
        case UserActionObjectType::Module:
        {
            Module* m = gNetlist->create_module(mGroupingName.toStdString());
            setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
        }
            break;
        }
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
            Grouping* grp = mObjectName.isEmpty()
                    ? gContentManager->getGroupingManagerWidget()->getModel()->addDefaultEntry()
                    : gNetlist->create_grouping(mObjectName.toStdString());
            setObject(UserActionObject(grp->get_id(),UserActionObjectType::Grouping));
        }
            break;
        default:
            break; // don't know how to create
        }
        UserAction::exec();
    }
}

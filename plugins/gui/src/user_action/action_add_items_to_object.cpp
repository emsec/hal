#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"

namespace hal
{
    ActionAddItemsToObjectFactory::ActionAddItemsToObjectFactory()
       : UserActionFactory("AddItemsToObject") {;}

    ActionAddItemsToObjectFactory* ActionAddItemsToObjectFactory::sFactory = new ActionAddItemsToObjectFactory;

    UserAction* ActionAddItemsToObjectFactory::newAction() const
    {
        return new ActionAddItemsToObject;
    }

    QString ActionAddItemsToObject::tagname() const
    {
        return ActionAddItemsToObjectFactory::sFactory->tagname();
    }

    void ActionAddItemsToObject::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData("mod",3);
        cryptoHash.addData(setToText(mModules).toUtf8());
        cryptoHash.addData("gat",3);
        cryptoHash.addData(setToText(mGates).toUtf8());
        cryptoHash.addData("net",3);
        cryptoHash.addData(setToText(mNets).toUtf8());
    }

    void ActionAddItemsToObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        if (!mModules.isEmpty()) xmlOut.writeTextElement("modules",setToText(mModules));
        if (!mGates.isEmpty())   xmlOut.writeTextElement("gates",setToText(mGates));
        if (!mNets.isEmpty())    xmlOut.writeTextElement("nets",setToText(mNets));
    }

    void ActionAddItemsToObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name()=="modules")
                mModules = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="gates")
                mGates = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="nets")
                mNets = setFromText(xmlIn.readElementText());
        }
    }

    void ActionAddItemsToObject::exec()
    {
        bool implemented = false;
        switch (mObject.type())
        {
        case UserActionObjectType::Module:
        {
            Module* m = gNetlist->get_module_by_id(mObject.id());
            if (m)
            {
                for (u32 id : mGates)
                    m->assign_gate(gNetlist->get_gate_by_id(id));
                for (u32 id : mModules)
                    gNetlist->get_module_by_id(id)->set_parent_module(m);
                implemented = true;
            }
        }
            break;
        case UserActionObjectType::Context:
        {
            GraphContext* ctx = gGraphContextManager->getContextById(mObject.id());
            if (ctx)
            {
                ctx->add(mModules, mGates);
                implemented = true;
            }

        }
            break;
        case UserActionObjectType::Grouping:
        {
            Grouping* grp = gNetlist->get_grouping_by_id(mObject.id());
            if (grp)
            {
                for (u32 id : mModules)
                    grp->assign_module_by_id(id);
                for (u32 id : mGates)
                    grp->assign_gate_by_id(id);
                for (u32 id : mNets)
                    grp->assign_net_by_id(id);
                implemented = true;
            }
        }
            break;
        default:
            break;
        }

        if (implemented)
        {
            mUndoAction = new ActionRemoveItemsFromObject(mModules,mGates,mNets);
            mUndoAction->setObject(mObject);
        }
        UserAction::exec();
    }
}

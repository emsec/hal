#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/user_action/action_create_object.h"

namespace hal
{
    ActionRemoveItemsFromObjectFactory::ActionRemoveItemsFromObjectFactory()
       : UserActionFactory("RemoveItemsFromObject") {;}

    ActionRemoveItemsFromObjectFactory* ActionRemoveItemsFromObjectFactory::sFactory = new ActionRemoveItemsFromObjectFactory;

    UserAction* ActionRemoveItemsFromObjectFactory::newAction() const
    {
        return new ActionRemoveItemsFromObject;
    }

    QString ActionRemoveItemsFromObject::tagname() const
    {
        return ActionRemoveItemsFromObjectFactory::sFactory->tagname();
    }

    void ActionRemoveItemsFromObject::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData("mod",3);
        cryptoHash.addData(setToText(mModules).toUtf8());
        cryptoHash.addData("gat",3);
        cryptoHash.addData(setToText(mGates).toUtf8());
        cryptoHash.addData("net",3);
        cryptoHash.addData(setToText(mNets).toUtf8());
        cryptoHash.addData("pin", 3);
        cryptoHash.addData(setToText(mPins).toUtf8());
    }

    void ActionRemoveItemsFromObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        writeParentObjectToXml(xmlOut);
        if (!mModules.isEmpty()) xmlOut.writeTextElement("modules",setToText(mModules));
        if (!mGates.isEmpty())   xmlOut.writeTextElement("gates",setToText(mGates));
        if (!mNets.isEmpty())    xmlOut.writeTextElement("nets",setToText(mNets));
        if (!mPins.isEmpty()) xmlOut.writeTextElement("pins",setToText(mPins));
    }

    void ActionRemoveItemsFromObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            readParentObjectFromXml(xmlIn);
            if (xmlIn.name()=="modules")
                mModules = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="gates")
                mGates = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="nets")
                mNets = setFromText(xmlIn.readElementText());
            else if (xmlIn.name()=="pins")
                mPins = setFromText(xmlIn.readElementText());
        }
    }

    bool ActionRemoveItemsFromObject::exec()
    {
        mUndoAction = nullptr;
        GraphContext* ctx;
        Grouping*     grp;
        switch (mObject.type())
        {
        case UserActionObjectType::Context:
            ctx = gGraphContextManager->getContextById(mObject.id());
            if (ctx)
            {
                ActionAddItemsToObject* undo =
                        new ActionAddItemsToObject(mModules,mGates,mNets);
                undo->setObject(mObject);
                PlacementHint plc(PlacementHint::GridPosition);
                for (u32 id : mModules)
                {
                    Node nd(id,Node::Module);
                    QPoint p = ctx->getLayouter()->nodeToPositionMap().value(nd);
                    plc.addGridPosition(nd,p);
                }
                for (u32 id : mGates)
                {
                    Node nd(id,Node::Gate);
                    QPoint p = ctx->getLayouter()->nodeToPositionMap().value(nd);
                    plc.addGridPosition(nd,p);
                }
                undo->setPlacementHint(plc);
                mUndoAction = undo;
                ctx->beginChange();
                ctx->remove(mModules, mGates);
                ctx->endChange();
            }
            else
                return false;
            break;
        case UserActionObjectType::Grouping:
            grp = gNetlist->get_grouping_by_id(mObject.id());
            if (grp)
            {
                for (u32 id : mModules)
                    grp->remove_module_by_id(id);
                for (u32 id : mGates)
                    grp->remove_gate_by_id(id);
                for (u32 id : mNets)
                    grp->remove_net_by_id(id);
            }
            else
                return false;
            break;
        case UserActionObjectType::PinGroup:
        {
            auto mod = gNetlist->get_module_by_id(mParentObject.id());
            if(mod)
            {
                auto pinGroupRes = mod->get_pin_group_by_id(mObject.id());
                if(pinGroupRes.is_error())
                    return false;

                for(u32 id : mPins)
                    if(mod->get_pin_by_id(id).is_error())
                        return false;
                for(u32 id : mPins)
                    mod->remove_pin_from_group(pinGroupRes.get(), mod->get_pin_by_id(id).get(), false);

                if(pinGroupRes.get()->empty())//delete group and undo=create+add
                {
                    UserActionCompound* act = new UserActionCompound;
                    act->setUseCreatedObject();
                    ActionCreateObject* actCreate = new ActionCreateObject(UserActionObjectType::PinGroup, QString::fromStdString(pinGroupRes.get()->get_name()));
                    actCreate->setParentObject(mParentObject);
                    act->addAction(actCreate);
                    act->addAction(new ActionAddItemsToObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), mPins));
                    mUndoAction = act;
                    mod->delete_pin_group(pinGroupRes.get());
                }
            }
            else
                return false;
        }
            break;
        default:
            return false;
        }

        if (!mUndoAction)
        {
            mUndoAction = new ActionAddItemsToObject(mModules,mGates,mNets,mPins);
            mUndoAction->setObject(mObject);
            mUndoAction->setParentObject(mParentObject);
        }
        return UserAction::exec();
    }
}

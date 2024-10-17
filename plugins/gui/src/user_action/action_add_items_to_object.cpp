#include "gui/user_action/action_add_items_to_object.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/layout_locker.h"
#include "gui/gui_globals.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_move_node.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/user_action/user_action_compound.h"
#include "hal_core/netlist/grouping.h"

namespace hal
{
    ActionAddItemsToObjectFactory::ActionAddItemsToObjectFactory() : UserActionFactory("AddItemsToObject")
    {
        ;
    }

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
        cryptoHash.addData("mod", 3);
        cryptoHash.addData(setToText(mModules).toUtf8());
        cryptoHash.addData("gat", 3);
        cryptoHash.addData(setToText(mGates).toUtf8());
        cryptoHash.addData("net", 3);
        cryptoHash.addData(setToText(mNets).toUtf8());
        cryptoHash.addData("pin", 3);
        cryptoHash.addData(setToText(mPins).toUtf8());
    }

    void ActionAddItemsToObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        writeParentObjectToXml(xmlOut);

        UserActionObjectType::ObjectType tp = UserActionObjectType::fromNodeType(mPlacementHint.preferredOrigin().type());
        switch (mPlacementHint.mode())
        {
        case PlacementHint::Standard:
            break;
        case PlacementHint::PreferLeft:
        case PlacementHint::PreferRight:
            xmlOut.writeStartElement("placement");
            xmlOut.writeAttribute("id", QString::number(mPlacementHint.preferredOrigin().id()));
            xmlOut.writeAttribute("type", UserActionObjectType::toString(tp));
            xmlOut.writeAttribute("mode", mPlacementHint.mode() == PlacementHint::PreferLeft ? "left" : "right");
            xmlOut.writeEndElement();
            break;
        case PlacementHint::GridPosition:
            xmlOut.writeTextElement("gridposition", gridToText(mPlacementHint.gridPosition()));
            break;
        }
        if (!mModules.isEmpty())
            xmlOut.writeTextElement("modules", setToText(mModules));
        if (!mGates.isEmpty())
            xmlOut.writeTextElement("gates", setToText(mGates));
        if (!mNets.isEmpty())
            xmlOut.writeTextElement("nets", setToText(mNets));
        if (!mPins.isEmpty())
            xmlOut.writeTextElement("pins", setToText(mPins));
    }

    void ActionAddItemsToObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "placement")
            {
                u32 id                                = xmlIn.attributes().value("id").toInt();
                UserActionObjectType::ObjectType tp   = UserActionObjectType::fromString(xmlIn.attributes().value("type").toString());
                PlacementHint::PlacementModeType mode = (xmlIn.attributes().value("mode").toString() == "left") ? PlacementHint::PreferLeft : PlacementHint::PreferRight;
                mPlacementHint                        = PlacementHint(mode, Node(id, UserActionObjectType::toNodeType(tp)));
                xmlIn.skipCurrentElement();    // no text body to read
            }
            else if (xmlIn.name() == "gridposition")
            {
                GridPlacement gp(gridFromText(xmlIn.readElementText()));
                mPlacementHint = PlacementHint(gp);
            }
            else if (xmlIn.name() == "modules")
                mModules = setFromText(xmlIn.readElementText());
            else if (xmlIn.name() == "gates")
                mGates = setFromText(xmlIn.readElementText());
            else if (xmlIn.name() == "nets")
                mNets = setFromText(xmlIn.readElementText());
            else if (xmlIn.name() == "pins")
                mPins = setFromText(xmlIn.readElementText());
        }
    }

    bool ActionAddItemsToObject::exec()
    {
        Module* m;
        GraphContext* ctx;
        Grouping* grp;
        mUndoAction = nullptr;

        LayoutLocker llock;

        switch (mObject.type())
        {
            case UserActionObjectType::Module:
                m = gNetlist->get_module_by_id(mObject.id());
                if (m)
                {
                    class ChildSet
                    {
                    public:
                        QSet<u32> mods;
                        QSet<u32> gats;
                        ActionAddItemsToObject* actionFactory(u32 moduleId) const
                        {
                            ActionAddItemsToObject* retval = new ActionAddItemsToObject(mods, gats);
                            retval->setObject(UserActionObject(moduleId, UserActionObjectType::Module));
                            return retval;
                        }
                    };
                    // hash parent module id and collect moved items to generate UNDO action
                    QHash<u32, ChildSet> parentHash;
                    GridPlacement* plc = nullptr;
                    const GraphContext* ctx = gContentManager->getContextManagerWidget()->getCurrentContext();
                    if (ctx) plc = ctx->getLayouter()->gridPlacementFactory();

                    std::vector<Gate*> gates;
                    for (u32 id : mGates)
                    {
                        Gate* g = gNetlist->get_gate_by_id(id);
                        Q_ASSERT(g);
                        parentHash[g->get_module()->get_id()].gats.insert(id);
                        gates.push_back(g);
                    }
                    m->assign_gates(gates);
                    for (u32 id : mModules)
                    {
                        Module* sm = gNetlist->get_module_by_id(id);
                        Q_ASSERT(sm);
                        u32 parentModuleId = sm->get_parent_module() ? sm->get_parent_module()->get_id() : 1;
                        parentHash[parentModuleId].mods.insert(id);
                        sm->set_parent_module(m);
                    }

                    if (!parentHash.isEmpty())
                    {
                        UserActionCompound* compound = new UserActionCompound;
                        for (auto it = parentHash.begin(); it != parentHash.end(); ++it)
                            compound->addAction(it.value().actionFactory(it.key()));
                        if (ctx && plc)
                        {
                            compound->addAction(new ActionMoveNode(ctx->id(),plc));
                            delete plc;
                        }
                        mUndoAction = compound;
                    }
                }
                else
                    return false;
                break;
            case UserActionObjectType::ContextView:
                ctx = gGraphContextManager->getContextById(mObject.id());
                if (ctx)
                {
                    ctx->beginChange();
                    ctx->add(mModules, mGates, mPlacementHint);
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
                        grp->assign_module_by_id(id, true);
                    for (u32 id : mGates)
                        grp->assign_gate_by_id(id, true);
                    for (u32 id : mNets)
                        grp->assign_net_by_id(id, true);
                }
                else
                    return false;
                break;
            default:
                return false;
        }

        if (!mUndoAction)
        {
            mUndoAction = new ActionRemoveItemsFromObject(mModules, mGates, mNets);
            mUndoAction->setObject(mObject);
        }

        return UserAction::exec();
    }
}    // namespace hal

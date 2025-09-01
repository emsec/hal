#include "gui/netlist_relay/netlist_relay.h"

#include "gui/file_manager/file_manager.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/module_model/module_item.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_color.h"
#include "gui/user_action/action_set_object_type.h"
#include "gui/user_action/action_move_node.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QInputDialog>
#include <functional>

namespace hal
{
    NetlistRelay::NetlistRelay(QObject* parent)
        : QObject(parent), mModuleColorManager(new ModuleColorManager(this))
    {
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &NetlistRelay::debugHandleFileOpened);    // DEBUG LINE
        connect(this, &NetlistRelay::signalThreadEvent, this, &NetlistRelay::handleThreadEvent, Qt::BlockingQueuedConnection);
    }

    NetlistRelay::~NetlistRelay()
    {
        unregisterNetlistCallbacks();
    }

    void NetlistRelay::unregisterNetlistCallbacks()
    {
        if (!gNetlist)
            return;    // no netlist -> no registered callbacks
        gNetlist->get_event_handler()->unregister_callback("gui_netlist_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_module_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_gate_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_net_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_grouping_handler");
    }

    void NetlistRelay::registerNetlistCallbacks()
    {
        gNetlist->get_event_handler()->register_callback(
            "gui_netlist_handler",
            std::function<void(NetlistEvent::event, Netlist*, u32)>(std::bind(&NetlistRelay::relayNetlistEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
            "gui_module_handler",
            std::function<void(ModuleEvent::event, Module*, u32)>(std::bind(&NetlistRelay::relayModuleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
            "gui_gate_handler", std::function<void(GateEvent::event, Gate*, u32)>(std::bind(&NetlistRelay::relayGateEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
            "gui_net_handler", std::function<void(NetEvent::event, Net*, u32)>(std::bind(&NetlistRelay::relayNetEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
            "gui_grouping_handler",
            std::function<void(GroupingEvent::event, Grouping*, u32)>(std::bind(&NetlistRelay::relayGroupingEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    }

    void NetlistRelay::handleNetlistModified()
    {
        if (!mNotified)
        {
            mNotified = true;
            gFileStatusManager->netlistChanged();
        }
    }

    QColor NetlistRelay::getModuleColor(const u32 id)
    {
        return mModuleColorManager->moduleColor(id);
    }

    ModuleColorManager* NetlistRelay::getModuleColorManager() const
    {
        return mModuleColorManager;
    }

    void NetlistRelay::changeElementNameDialog(ModuleItem::TreeItemType type, u32 id)
    {
        QString oldName;
        QString prompt;
        UserActionObject uao;

        if (type == ModuleItem::TreeItemType::Module)
        {
            Module* m = gNetlist->get_module_by_id(id);
            assert(m);
            oldName   = QString::fromStdString(m->get_name());
            prompt    = "Change module name";
            uao = UserActionObject(id, UserActionObjectType::Module);
        }
        else if (type == ModuleItem::TreeItemType::Gate)
        {
            Gate* g   = gNetlist->get_gate_by_id(id);
            assert(g);
            oldName   = QString::fromStdString(g->get_name());
            prompt    = "Change gate name";
            uao = UserActionObject(id, UserActionObjectType::Gate);
        }
        else if (type == ModuleItem::TreeItemType::Net)
        {
            Net* n    = gNetlist->get_net_by_id(id);
            assert(n);
            oldName   = QString::fromStdString(n->get_name());
            prompt    = "Change net name";
            uao = UserActionObject(id, UserActionObjectType::Net);
        }
        else return;

        bool confirm;
        QString newName =
                QInputDialog::getText(nullptr, prompt, "New name:", QLineEdit::Normal,
                                      oldName, &confirm);
        if (confirm)
        {
            ActionRenameObject* act = new ActionRenameObject(newName);
            act->setObject(uao);
            act->exec();
        }
    }

    void NetlistRelay::changeModuleTypeDialog(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        bool ok;
        QString text = QInputDialog::getText(nullptr, "Change Module Type", "New Type", QLineEdit::Normal, QString::fromStdString(m->get_type()), &ok);

        if (ok)
        {
            ActionSetObjectType* act = new ActionSetObjectType(text);
            act->setObject(UserActionObject(id, UserActionObjectType::Module));
            act->exec();
        }
    }

    void NetlistRelay::changeModuleColorDialog(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        QColor color = QColorDialog::getColor();

        if (!color.isValid())
            return;

        ActionSetObjectColor* act = new ActionSetObjectColor(color);
        act->setObject(UserActionObject(id, UserActionObjectType::Module));
        act->exec();
    }

    void NetlistRelay::addToModuleDialog(const Node &node)
    {
        // prepare set of content, find first (reference-) node
        Node firstNode = node;
        QSet<u32> gatIds;
        QSet<u32> modIds;

        GraphContext* context = gContentManager->getContextManagerWidget()->getCurrentContext();
        Q_ASSERT(context);

        // exclusive module view will update automatically if module elements get moved
        bool specialUpdateRequired = !context->getExclusiveModuleId();

        if (node.isNull())
        {
            for (u32 id : gSelectionRelay->selectedGatesList())
            {
                if (specialUpdateRequired && ! context->gates().contains(id))
                    specialUpdateRequired = false;
                if (firstNode.isNull()) firstNode = Node(id,Node::Gate);
                gatIds.insert(id);
            }

            for (u32 id : gSelectionRelay->selectedModulesList())
            {
                if (specialUpdateRequired && ! context->modules().contains(id))
                    specialUpdateRequired = false;
                if (firstNode.isNull()) firstNode = Node(id,Node::Module);
                modIds.insert(id);
            }
        }
        else if (node.isModule())
            modIds.insert(node.id());
        else
            gatIds.insert(node.id());
        if (firstNode.isNull()) return; // nothing to move

        // find common parent, if nullptr top_level was selected => abort
        std::unordered_set<Gate*>   gatsContent;
        for (u32 id : gatIds)
            gatsContent.insert(gNetlist->get_gate_by_id(id));

        std::unordered_set<Module*> modsContent;
        for (u32 id : modIds)
            modsContent.insert(gNetlist->get_module_by_id(id));

        Module* parentModule  = gui_utility::firstCommonAncestor(modsContent, gatsContent);
        if (!parentModule) return;
        QString parentName = QString::fromStdString(parentModule->get_name());

        ModuleDialog md({},"Move to module",false,nullptr,qApp->activeWindow());
        if (md.exec() != QDialog::Accepted) return;


        UserActionCompound* compound = new UserActionCompound;
        compound->setUseCreatedObject();

        if (md.isNewModule())
        {
            bool ok;
            QString name = QInputDialog::getText(nullptr, "",
                                                 "New module will be created under \"" + parentName + "\"\nModule Name:",
                                                 QLineEdit::Normal, "", &ok);
            if (!ok || name.isEmpty()) return;
            ActionCreateObject* actNewModule = new ActionCreateObject(UserActionObjectType::Module, name);
            actNewModule->setParentId(parentModule->get_id());
            compound->addAction(actNewModule);
            compound->addAction(new ActionAddItemsToObject(modIds,gatIds));
        }
        else
        {
            u32 targetModuleId = md.selectedId();
            ActionAddItemsToObject* actAddItems = new ActionAddItemsToObject(modIds,gatIds);
            actAddItems->setObject(UserActionObject(targetModuleId,UserActionObjectType::Module));
            compound->addAction(actAddItems);
            specialUpdateRequired = false;
        }

        // move module to position of first content node
        const NodeBox* box = context->getLayouter()->boxes().boxForNode(firstNode);
        if (box && (specialUpdateRequired || context->getExclusiveModuleId()))
        {
            ActionMoveNode* actMoveNode = new ActionMoveNode(context->id(), QPoint(box->x(),box->y()));
            compound->addAction(actMoveNode);
        }

        if (specialUpdateRequired)
        {
            context->setSpecialUpdate(true);
            context->setScheduleRemove(modIds,gatIds);
        }

        compound->exec();

        // update selection
        gSelectionRelay->clear();
        gSelectionRelay->addModule(compound->object().id());
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Module,compound->object().id());
        gSelectionRelay->relaySelectionChanged(this);
        gContentManager->getGraphTabWidget()->ensureSelectionVisible();
    }

    void NetlistRelay::addChildModuleDialog(const u32 id)
    {
        // NOT THREADSAFE

        bool ok;
        QString name = QInputDialog::getText(nullptr, "", "Module Name:", QLineEdit::Normal, "", &ok);

        if (!ok || name.isEmpty())
            return;

        Module* m = gNetlist->get_module_by_id(id);

        if (!m)
            return;

        ActionCreateObject* act = new ActionCreateObject(UserActionObjectType::Module, name);
        act->setParentId(id);
        act->exec();
    }

    void NetlistRelay::deleteModule(const u32 id)
    {
        ActionDeleteObject* delMod = new ActionDeleteObject;
        delMod->setObject(UserActionObject(id, UserActionObjectType::Module));

        GraphContext* ctx = gGraphContextManager->getContextByExclusiveModuleId(id);
        if (ctx)
        {
            // module exclusively connected to context, so delete context too
            UserActionCompound* compnd = new UserActionCompound;
            ActionDeleteObject* delCtx = new ActionDeleteObject;
            delCtx->setObject(UserActionObject(ctx->id(), UserActionObjectType::ContextView));
            compnd->addAction(delCtx);
            compnd->addAction(delMod);
            compnd->exec();
        }
        else
        {
            delMod->exec();
        }
    }

    void NetlistRelay::reset()
    {
        mNotified = false;
    }

    void NetlistRelay::relayNetlistEvent(NetlistEvent::event ev, Netlist* object, u32 associated_data)
    {
        if (!object)
            return;    // SHOULD NEVER BE REACHED

        if (object != gNetlist)
            return;

        handleNetlistModified();

        if (dynamic_cast<PythonThread*>(QThread::currentThread()))
        {
            Q_EMIT signalThreadEvent(TetNetlist, (int)ev, object, associated_data);
            qApp->processEvents();
            return;
        }

        switch (ev)
        {
            case NetlistEvent::event::id_changed: {
                ///< associated_data = old id

                Q_EMIT netlistIdChanged(object, associated_data);
                break;
            }
            case NetlistEvent::event::input_filename_changed: {
                ///< no associated_data

                Q_EMIT netlistInputFilenameChanged(object);
                break;
            }
            case NetlistEvent::event::design_name_changed: {
                ///< no associated_data

                Q_EMIT netlistDesignNameChanged(object);
                break;
            }
            case NetlistEvent::event::device_name_changed: {
                ///< no associated_data

                Q_EMIT netlistDeviceNameChanged(object);
                break;
            }
            case NetlistEvent::event::marked_global_vcc: {
                ///< associated_data = id of gate

                Q_EMIT netlistMarkedGlobalVcc(object, associated_data);
                break;
            }
            case NetlistEvent::event::marked_global_gnd: {
                ///< associated_data = id of gate

                Q_EMIT netlistMarkedGlobalGnd(object, associated_data);
                break;
            }
            case NetlistEvent::event::unmarked_global_vcc: {
                ///< associated_data = id of gate

                Q_EMIT netlistUnmarkedGlobalVcc(object, associated_data);
                break;
            }
            case NetlistEvent::event::unmarked_global_gnd: {
                ///< associated_data = id of gate

                Q_EMIT netlistUnmarkedGlobalGnd(object, associated_data);
                break;
            }
            case NetlistEvent::event::marked_global_input: {
                ///< associated_data = id of net
                gGraphContextManager->handleMarkedGlobalInput(associated_data);

                Q_EMIT netlistMarkedGlobalInput(object, associated_data);
                break;
            }
            case NetlistEvent::event::marked_global_output: {
                ///< associated_data = id of net
                gGraphContextManager->handleMarkedGlobalOutput(associated_data);

                Q_EMIT netlistMarkedGlobalOutput(object, associated_data);
                break;
            }
            case NetlistEvent::event::unmarked_global_input: {
                ///< associated_data = id of net
                gGraphContextManager->handleUnmarkedGlobalInput(associated_data);

                Q_EMIT netlistUnmarkedGlobalInput(object, associated_data);
                break;
            }
            case NetlistEvent::event::unmarked_global_output: {
                ///< associated_data = id of net
                gGraphContextManager->handleUnmarkedGlobalOutput(associated_data);

                Q_EMIT netlistUnmarkedGlobalOutput(object, associated_data);
                break;
            }
        }
    }

    void NetlistRelay::relayGroupingEvent(GroupingEvent::event ev, Grouping* grp, u32 associated_data)
    {
        if (!grp)
            return;    // SHOULD NEVER BE REACHED

        if (grp->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        if (dynamic_cast<PythonThread*>(QThread::currentThread()))
        {
            Q_EMIT signalThreadEvent(TetGrouping, (int)ev, grp, associated_data);
            qApp->processEvents();
            return;
        }

        switch (ev)
        {
            case GroupingEvent::event::created:
                Q_EMIT groupingCreated(grp);
                break;
            case GroupingEvent::event::removed:
                Q_EMIT groupingRemoved(grp);
                break;
            case GroupingEvent::event::name_changed:
                Q_EMIT groupingNameChanged(grp);
                break;
            case GroupingEvent::event::color_changed:
                Q_EMIT groupingColorChanged(grp);
                break;
            case GroupingEvent::event::gate_assigned:
                Q_EMIT groupingGateAssigned(grp, associated_data);
                break;
            case GroupingEvent::event::gate_removed:
                Q_EMIT groupingGateRemoved(grp, associated_data);
                break;
            case GroupingEvent::event::net_assigned:
                Q_EMIT groupingNetAssigned(grp, associated_data);
                break;
            case GroupingEvent::event::net_removed:
                Q_EMIT groupingNetRemoved(grp, associated_data);
                break;
            case GroupingEvent::event::module_assigned:
                Q_EMIT groupingModuleAssigned(grp, associated_data);
                break;
            case GroupingEvent::event::module_removed:
                Q_EMIT groupingModuleRemoved(grp, associated_data);
                break;
        }
    }

    void NetlistRelay::relayModuleEvent(ModuleEvent::event ev, Module* mod, u32 associated_data)
    {
        if (!mod)
            return;    // SHOULD NEVER BE REACHED

        if (mod->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        if (dynamic_cast<PythonThread*>(QThread::currentThread()))
        {
            Q_EMIT signalThreadEvent(TetModule, (int)ev, mod, associated_data);
            qApp->processEvents();
            return;
        }

        switch (ev)
        {
            case ModuleEvent::event::created: {
                //< no associated_data

                // suppress actions if we receive this for the top module
                if (mod->get_parent_module() != nullptr)
                {
                    mModuleColorManager->setRandomColor(mod->get_id());
                }

                gGraphContextManager->handleModuleCreated(mod);

                Q_EMIT moduleCreated(mod);
                break;
            }
            case ModuleEvent::event::removed: {
                //< no associated_data

                mModuleColorManager->removeColor(mod->get_id());

                gGraphContextManager->handleModuleRemoved(mod);
                gSelectionRelay->handleModuleRemoved(mod->get_id());

                Q_EMIT moduleRemoved(mod);
                break;
            }
            case ModuleEvent::event::name_changed: {
                //< no associated_data

                gGraphContextManager->handleModuleNameChanged(mod);

                Q_EMIT moduleNameChanged(mod);
                break;
            }
            case ModuleEvent::event::parent_changed: {
                //< no associated_data

                Q_EMIT moduleParentChanged(mod);
                break;
            }
            case ModuleEvent::event::submodule_added: {
                //< associated_data = id of added module

                gGraphContextManager->handleModuleSubmoduleAdded(mod, associated_data);

                Q_EMIT moduleSubmoduleAdded(mod, associated_data);
                break;
            }
            case ModuleEvent::event::submodule_removed: {
                //< associated_data = id of removed module

                gGraphContextManager->handleModuleSubmoduleRemoved(mod, associated_data);

                Q_EMIT moduleSubmoduleRemoved(mod, associated_data);
                break;
            }
            case ModuleEvent::event::gate_assigned: {
                //< associated_data = id of inserted gate

                gGraphContextManager->handleModuleGateAssigned(mod, associated_data);

                Q_EMIT moduleGateAssigned(mod, associated_data);
                break;
            }
            case ModuleEvent::event::gate_removed: {
                //< associated_data = id of removed gate

                gGraphContextManager->handleModuleGateRemoved(mod, associated_data);

                Q_EMIT moduleGateRemoved(mod, associated_data);
                break;
            }
            case ModuleEvent::event::pin_changed: {
                 //< associated_data = [4LSB: type of action]  [28HSB: id of pin group or pin]
                PinEvent pev = (PinEvent) (associated_data&0xF);
                u32 id = (associated_data >> 4);

                /* Dump pin event for debugging
                std::cerr << "Pin Event <" << enum_to_string<PinEvent>(pev) << "> ID=" << id << "\n";
                for (PinGroup<ModulePin>* pg : mod->get_pin_groups())
                {
                    std::cerr << "  " << pg->get_start_index() << " " << pg->get_name() << " [" << pg->get_id() << "]"
                              << (pg->is_ascending() ? " asc\n" : " desc\n");
                    for (ModulePin* pin : pg->get_pins())
                        std::cerr << "    " << pg->get_index(pin).get() << " " << pin->get_name() << "\n";
                }
                std::cerr << "---------------------" << std::endl;
                */

                gGraphContextManager->handleModulePortsChanged(mod,pev,id);

                Q_EMIT modulePortsChanged(mod,pev,id);
                break;
            }
            case ModuleEvent::event::type_changed: {
                //< no associated_data

                gGraphContextManager->handleModuleTypeChanged(mod);

                Q_EMIT moduleTypeChanged(mod);
                break;
            }
            case ModuleEvent::event::gates_assign_begin: {
                //< associated_data = number of gates

                Q_EMIT moduleGatesAssignBegin(mod, associated_data);
                break;
            }
            case ModuleEvent::event::gates_assign_end: {
                //< associated_data = number of gates

                Q_EMIT moduleGatesAssignEnd(mod, associated_data);
                break;
            }
            case ModuleEvent::event::gates_remove_begin: {
                //< associated_data = number of gates

                Q_EMIT moduleGatesRemoveBegin(mod, associated_data);
                break;
            }
            case ModuleEvent::event::gates_remove_end: {
                //< associated_data = number of gates

                Q_EMIT moduleGatesRemoveEnd(mod, associated_data);
                break;
            }
        }
    }

    void NetlistRelay::relayGateEvent(GateEvent::event ev, Gate* gat, u32 associated_data)
    {
        UNUSED(associated_data);
        if (!gat)
            return;    // SHOULD NEVER BE REACHED

        if (gat->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        if (dynamic_cast<PythonThread*>(QThread::currentThread()))
        {
            Q_EMIT signalThreadEvent(TetGate, (int)ev, gat, associated_data);
            qApp->processEvents();
            return;
        }

        switch (ev)
        {
            case GateEvent::event::created: {
                //< no associated_data

                Q_EMIT gateCreated(gat);
                break;
            }
            case GateEvent::event::removed: {
                //< no associated_data

                gSelectionRelay->handleGateRemoved(gat->get_id());

                gGraphContextManager->handleGateRemoved(gat);

                Q_EMIT gateRemoved(gat);
                break;
            }
            case GateEvent::event::name_changed: {
                //< no associated_data

                gGraphContextManager->handleGateNameChanged(gat);

                Q_EMIT gateNameChanged(gat);
                break;
            }
            case GateEvent::event::boolean_function_changed: {
                //< no associated_data

                Q_EMIT gateBooleanFunctionChanged(gat);
                break;
            }
            case GateEvent::event::location_changed: {
                //< no associated_data

                Q_EMIT gateLocationChanged(gat);
                break;
            }
            default:
                break;
        }
    }

    void NetlistRelay::relayNetEvent(NetEvent::event ev, Net* net, u32 associated_data)
    {
        if (!net)
            return;    // SHOULD NEVER BE REACHED

        if (net->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        if (dynamic_cast<PythonThread*>(QThread::currentThread()))
        {
            Q_EMIT signalThreadEvent(TetNet, (int)ev, net, associated_data);
            qApp->processEvents();
            return;
        }

        switch (ev)
        {
            case NetEvent::event::created: {
                //< no associated_data

                gGraphContextManager->handleNetCreated(net);

                Q_EMIT netCreated(net);
                break;
            }
            case NetEvent::event::removed: {
                //< no associated_data

                gGraphContextManager->handleNetRemoved(net);
                gSelectionRelay->handleNetRemoved(net->get_id());

                Q_EMIT netRemoved(net);
                break;
            }
            case NetEvent::event::name_changed: {
                //< no associated_data

                gGraphContextManager->handleNetNameChanged(net);

                Q_EMIT netNameChanged(net);
                break;
            }
                // FIXME add src_added, src_removed
                // case NetEvent::event::src_changed:
                // {
                //     //< no associated_data

                //     gGraphContextManager->handle_net_source_changed(object);

                //     Q_EMIT net_source_changed(object);
                //     break;
                // }
            case NetEvent::event::src_added: {
                //< associated_data = id of src gate

                gGraphContextManager->handleNetSourceAdded(net, associated_data);

                Q_EMIT netSourceAdded(net, associated_data);
                break;
            }
            case NetEvent::event::src_removed: {
                //< associated_data = id of src gate

                gGraphContextManager->handleNetSourceRemoved(net, associated_data);

                Q_EMIT netSourceRemoved(net, associated_data);
                break;
            }
            case NetEvent::event::dst_added: {
                //< associated_data = id of dst gate

                gGraphContextManager->handleNetDestinationAdded(net, associated_data);

                Q_EMIT netDestinationAdded(net, associated_data);
                break;
            }
            case NetEvent::event::dst_removed: {
                //< associated_data = id of dst gate

                gGraphContextManager->handleNetDestinationRemoved(net, associated_data);

                Q_EMIT netDestinationRemoved(net, associated_data);
                break;
            }
        }
    }

    void NetlistRelay::handleThreadEvent(int type, int evt, void* object, u32 associated_data)
    {
        switch (type)
        {
            case TetNetlist:
                //            qDebug() << "Evt nlst" << evt << associated_data;
                relayNetlistEvent((NetlistEvent::event)evt, static_cast<Netlist*>(object), associated_data);
                break;
            case TetModule:
                //            qDebug() << "Evt modu" << evt << static_cast<Module*>(object)->get_id() << associated_data;
                relayModuleEvent((ModuleEvent::event)evt, static_cast<Module*>(object), associated_data);
                break;
            case TetGate:
                //            qDebug() << "Evt gate" << evt << static_cast<Gate*>(object)->get_id() << associated_data;
                relayGateEvent((GateEvent::event)evt, static_cast<Gate*>(object), associated_data);
                break;
            case TetNet:
                //            qDebug() << "Evt net_" << evt << static_cast<Net*>(object)->get_id() << associated_data;
                relayNetEvent((NetEvent::event)evt, static_cast<Net*>(object), associated_data);
                break;
            case TetGrouping:
                //            qDebug() << "Evt grup" << evt << static_cast<Grouping*>(object)->get_id() << associated_data;
                relayGroupingEvent((GroupingEvent::event)evt, static_cast<Grouping*>(object), associated_data);
                break;
        }
    }

    void NetlistRelay::dumpModuleRecursion(Module *m)
    {
        for (int i=0; i<m->get_submodule_depth(); i++)
            std::cerr << "   ";
        std::cerr << "Mod " << m->get_id() << " <" << m->get_name() << ">\n";
        for (Module* sm : m->get_submodules())
            dumpModuleRecursion(sm);
    }

    void NetlistRelay::debugHandleFileOpened()
    {
        for (Module* m : gNetlist->get_modules())
            mModuleColorManager->setRandomColor(m->get_id());
        mColorSerializer.restore(mModuleColorManager);
    }

    void NetlistRelay::debugHandleFileClosed()
    {
    }
}    // namespace hal

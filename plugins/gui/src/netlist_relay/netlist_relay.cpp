#include "gui/netlist_relay/netlist_relay.h"

#include "hal_core/utilities/log.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"

#include "gui/module_model/module_item.h"
#include "gui/module_model/module_model.h"
#include "gui/file_manager/file_manager.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_color.h"
#include "gui/user_action/action_set_object_type.h"

#include <QColorDialog>
#include <QDebug>
#include <QInputDialog>

#include <functional>

namespace hal
{
    NetlistRelay::NetlistRelay(QObject* parent) : QObject(parent), mModuleModel(new ModuleModel(this))
    {
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &NetlistRelay::debugHandleFileOpened);    // DEBUG LINE
    }

    NetlistRelay::~NetlistRelay()
    {
        unregisterNetlistCallbacks();
    }

    void NetlistRelay::unregisterNetlistCallbacks()
    {
        if (!gNetlist) return; // no netlist -> no registered callbacks
        log_info("test", "unregister netlist callbacks");
        gNetlist->get_event_handler()->unregister_callback("gui_netlist_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_module_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_gate_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_net_handler");
        gNetlist->get_event_handler()->unregister_callback("gui_grouping_handler");
    }

    void NetlistRelay::registerNetlistCallbacks()
    {
        log_info("test", "register netlist callbacks");
        gNetlist->get_event_handler()->register_callback("gui_netlist_handler",
                                                 std::function<void(NetlistEvent::event, Netlist*, u32)>(
                                                     std::bind(&NetlistRelay::relayNetlistEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback("gui_module_handler",
                                                std::function<void(ModuleEvent::event, Module*, u32)>(
                                                    std::bind(&NetlistRelay::relayModuleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback("gui_gate_handler",
                                              std::function<void(GateEvent::event, Gate*, u32)>(
                                                  std::bind(&NetlistRelay::relayGateEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback("gui_net_handler",
                                             std::function<void(NetEvent::event, Net*, u32)>(
                                                 std::bind(&NetlistRelay::relayNetEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback("gui_grouping_handler",
                                                std::function<void(GroupingEvent::event, Grouping*, u32)>(
                                                      std::bind(&NetlistRelay::relayGroupingEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
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
        return mModuleModel->moduleColor(id);
    }

    ModuleModel* NetlistRelay::getModuleModel()
    {
        return mModuleModel;
    }

    void NetlistRelay::changeModuleName(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        bool ok;
        QString text = QInputDialog::getText(nullptr, "Rename Module", "New Name", QLineEdit::Normal, QString::fromStdString(m->get_name()), &ok);

        if (ok && !text.isEmpty())
        {
            ActionRenameObject* act = new ActionRenameObject(text);
            act->setObject(UserActionObject(id,UserActionObjectType::Module));
            act->exec();
        }
    }

    void NetlistRelay::changeModuleType(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        bool ok;
        QString text = QInputDialog::getText(nullptr, "Change Module Type", "New Type", QLineEdit::Normal, QString::fromStdString(m->get_type()), &ok);

        if (ok && !text.isEmpty())
        {
            ActionSetObjectType* act = new ActionSetObjectType(text);
            act->setObject(UserActionObject(id,UserActionObjectType::Module));
            act->exec();
        }
    }

    void NetlistRelay::changeModuleColor(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        QColor color = QColorDialog::getColor();

        if (!color.isValid())
            return;

        ActionSetObjectColor* act = new ActionSetObjectColor(color);
        act->setObject(UserActionObject(id,UserActionObjectType::Module));
        act->exec();

        Q_EMIT moduleColorChanged(m);
    }

    void NetlistRelay::addSelectionToModule(const u32 id)
    {
        // NOT THREADSAFE
        // DECIDE HOW TO HANDLE MODULES

        ActionAddItemsToObject* act = new ActionAddItemsToObject({},gSelectionRelay->selectedGates());
        act->setObject(UserActionObject(id,UserActionObjectType::Module));
        act->exec();
    }

    void NetlistRelay::addChildModule(const u32 id)
    {
        // NOT THREADSAFE

        bool ok;
        QString name = QInputDialog::getText(nullptr, "", "Module Name:", QLineEdit::Normal, "", &ok);

        if (!ok || name.isEmpty())
            return;

        Module* m = gNetlist->get_module_by_id(id);

        if (!m)
            return;

        ActionCreateObject* act = new ActionCreateObject(UserActionObjectType::Module,name);
        act->setParentId(id);
        act->exec();
    }

    void NetlistRelay::deleteModule(const u32 id)
    {
        ActionDeleteObject* act = new ActionDeleteObject;
        act->setObject(UserActionObject(id,UserActionObjectType::Module));
        act->exec();
    }

    void NetlistRelay::reset()
    {
        mNotified = false;
    }

    void NetlistRelay::relayNetlistEvent(NetlistEvent::event ev, Netlist* object, u32 associated_data)
    {
        if (!object)
            return;    // SHOULD NEVER BE REACHED

        if(object != gNetlist)
            return;

        handleNetlistModified();

        switch (ev)
        {
        case NetlistEvent::event::id_changed:
        {
            ///< associated_data = old id

            Q_EMIT netlistIdChanged(object, associated_data);
            break;
        }
        case NetlistEvent::event::input_filename_changed:
        {
            ///< no associated_data

            Q_EMIT netlistInputFilenameChanged(object);
            break;
        }
        case NetlistEvent::event::design_name_changed:
        {
            ///< no associated_data

            Q_EMIT netlistDesignNameChanged(object);
            break;
        }
        case NetlistEvent::event::device_name_changed:
        {
            ///< no associated_data

            Q_EMIT netlistDeviceNameChanged(object);
            break;
        }
        case NetlistEvent::event::marked_global_vcc:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistMarkedGlobalVcc(object, associated_data);
            break;
        }
        case NetlistEvent::event::marked_global_gnd:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistMarkedGlobalGnd(object, associated_data);
            break;
        }
        case NetlistEvent::event::unmarked_global_vcc:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistUnmarkedGlobalVcc(object, associated_data);
            break;
        }
        case NetlistEvent::event::unmarked_global_gnd:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistUnmarkedGlobalGnd(object, associated_data);
            break;
        }
        case NetlistEvent::event::marked_global_input:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleMarkedGlobalInput(associated_data);

            Q_EMIT netlistMarkedGlobalInput(object, associated_data);
            break;
        }
        case NetlistEvent::event::marked_global_output:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleMarkedGlobalOutput(associated_data);

            Q_EMIT netlistMarkedGlobalOutput(object, associated_data);
            break;
        }
        case NetlistEvent::event::marked_global_inout:
        {
            ///< associated_data = id of net

            Q_EMIT netlistMarkedGlobalInout(object, associated_data);
            break;
        }
        case NetlistEvent::event::unmarked_global_input:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleUnmarkedGlobalInput(associated_data);

            Q_EMIT netlistUnmarkedGlobalInput(object, associated_data);
            break;
        }
        case NetlistEvent::event::unmarked_global_output:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleUnmarkedGlobalOutput(associated_data);

            Q_EMIT netlistUnmarkedGlobalOutput(object, associated_data);
            break;
        }
        case NetlistEvent::event::unmarked_global_inout:
        {
            ///< associated_data = id of net

            Q_EMIT netlistUnmarkedGlobalInout(object, associated_data);
            break;
        }
        }
    }

    void NetlistRelay::relayGroupingEvent(GroupingEvent::event ev, Grouping *grp, u32 associated_data)
    {

        if (!grp)
            return;    // SHOULD NEVER BE REACHED

        if(grp->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

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
        case GroupingEvent::event::gate_assigned:
            Q_EMIT groupingGateAssigned(grp,associated_data);
            break;
        case GroupingEvent::event::gate_removed:
            Q_EMIT groupingGateRemoved(grp,associated_data);
            break;
        case GroupingEvent::event::net_assigned:
            Q_EMIT groupingNetAssigned(grp,associated_data);
            break;
        case GroupingEvent::event::net_removed:
            Q_EMIT groupingNetRemoved(grp,associated_data);
            break;
        case GroupingEvent::event::module_assigned:
            Q_EMIT groupingModuleAssigned(grp,associated_data);
            break;
        case GroupingEvent::event::module_removed:
            Q_EMIT groupingModuleRemoved(grp,associated_data);
            break;
        }
    }

    void NetlistRelay::relayModuleEvent(ModuleEvent::event ev, Module* mod, u32 associated_data)
    {
        if (!mod)
            return;    // SHOULD NEVER BE REACHED

        if(mod->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        switch (ev)
        {
        case ModuleEvent::event::created:
        {
            //< no associated_data

            // suppress actions if we receive this for the top module
            if (mod->get_parent_module() != nullptr)
            {
                mModuleModel->setRandomColor(mod->get_id());
            }

            Q_EMIT moduleCreated(mod);
            break;
        }
        case ModuleEvent::event::removed:
        {
            //< no associated_data

            mModuleModel->removeColor(mod->get_id());

            gGraphContextManager->handleModuleRemoved(mod);
            gSelectionRelay->handleModuleRemoved(mod->get_id());

            Q_EMIT moduleRemoved(mod);
            break;
        }
        case ModuleEvent::event::name_changed:
        {
            //< no associated_data

            mModuleModel->updateModule(mod->get_id());

            gGraphContextManager->handleModuleNameChanged(mod);

            Q_EMIT moduleNameChanged(mod);
            break;
        }
        case ModuleEvent::event::parent_changed:
        {
            //< no associated_data

            Q_EMIT moduleParentChanged(mod);
            break;
        }
        case ModuleEvent::event::submodule_added:
        {
            //< associated_data = id of added module

            mModuleModel->addModule(associated_data, mod->get_id());

            gGraphContextManager->handleModuleSubmoduleAdded(mod, associated_data);

            Q_EMIT moduleSubmoduleAdded(mod, associated_data);
            break;
        }
        case ModuleEvent::event::submodule_removed:
        {
            //< associated_data = id of removed module

            mModuleModel->remove_module(associated_data);

            gGraphContextManager->handleModuleSubmoduleRemoved(mod, associated_data);

            Q_EMIT moduleSubmoduleRemoved(mod, associated_data);
            break;
        }
        case ModuleEvent::event::gate_assigned:
        {
            //< associated_data = id of inserted gate

            gGraphContextManager->handleModuleGateAssigned(mod, associated_data);

            Q_EMIT moduleGateAssigned(mod, associated_data);
            break;
        }
        case ModuleEvent::event::gate_removed:
        {
            //< associated_data = id of removed gate

            gGraphContextManager->handleModuleGateRemoved(mod, associated_data);

            Q_EMIT moduleGateRemoved(mod, associated_data);
            break;
        }
        case ModuleEvent::event::input_port_name_changed:
        {
            //< associated data = respective net

            gGraphContextManager->handleModuleInputPortNameChanged(mod, associated_data);

            Q_EMIT moduleInputPortNameChanged(mod, associated_data);
            break;
        }
        case ModuleEvent::event::output_port_name_changed:
        {
            //< associated data = respective net

            gGraphContextManager->handleModuleOutputPortNameChanged(mod, associated_data);

            Q_EMIT moduleOutputPortNameChanged(mod, associated_data);
            break;
        }
        case ModuleEvent::event::type_changed:
        {
            //< no associated_data

            gGraphContextManager->handleModuleTypeChanged(mod);

            Q_EMIT moduleTypeChanged(mod);
            break;
        }
        }
    }

    void NetlistRelay::relayGateEvent(GateEvent::event ev, Gate* gat, u32 associated_data)
    {
        UNUSED(associated_data);
        if (!gat)
            return;    // SHOULD NEVER BE REACHED

        if(gat->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        switch (ev)
        {
        case GateEvent::event::created:
        {
            //< no associated_data

            Q_EMIT gateCreated(gat);
            break;
        }
        case GateEvent::event::removed:
        {
            //< no associated_data

           gSelectionRelay->handleGateRemoved(gat->get_id());

            Q_EMIT gateRemoved(gat);
            break;
        }
        case GateEvent::event::name_changed:
        {
            //< no associated_data

            gGraphContextManager->handleGateNameChanged(gat);

            Q_EMIT gateNameChanged(gat);
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

        if(net->get_netlist() != gNetlist)
            return;

        handleNetlistModified();

        switch (ev)
        {
        case NetEvent::event::created:
        {
            //< no associated_data

            gGraphContextManager->handleNetCreated(net);

            Q_EMIT netCreated(net);
            break;
        }
        case NetEvent::event::removed:
        {
            //< no associated_data

            gGraphContextManager->handleNetRemoved(net);
            gSelectionRelay->handleNetRemoved(net->get_id());

            Q_EMIT netRemoved(net);
            break;
        }
        case NetEvent::event::name_changed:
        {
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
        case NetEvent::event::src_added:
        {
            //< associated_data = id of src gate

            gGraphContextManager->handleNetSourceAdded(net, associated_data);

            Q_EMIT netSourceAdded(net, associated_data);
            break;
        }
        case NetEvent::event::src_removed:
        {
            //< associated_data = id of src gate

            gGraphContextManager->handleNetSourceRemoved(net, associated_data);

            Q_EMIT netSourceRemoved(net, associated_data);
            break;
        }
        case NetEvent::event::dst_added:
        {
            //< associated_data = id of dst gate

            gGraphContextManager->handleNetDestinationAdded(net, associated_data);

            Q_EMIT netDestinationAdded(net, associated_data);
            break;
        }
        case NetEvent::event::dst_removed:
        {
            //< associated_data = id of dst gate

            gGraphContextManager->handleNetDestinationRemoved(net, associated_data);

            Q_EMIT netDestinationRemoved(net, associated_data);
            break;
        }
        }
    }

    void NetlistRelay::debugHandleFileOpened()
    {
        for (Module* m : gNetlist->get_modules())
            mModuleModel->setRandomColor(m->get_id());
        mModuleModel->init();
    }

    void NetlistRelay::debugHandleFileClosed()
    {
        mModuleModel->clear();
    }
}

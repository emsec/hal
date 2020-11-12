#include "gui/netlist_relay/netlist_relay.h"

#include "hal_core/utilities/log.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"

#include "gui/module_model/module_item.h"
#include "gui/module_model/module_model.h"

#include "gui/file_manager/file_manager.h"    // DEBUG LINE
#include "gui/gui_globals.h"                  // DEBUG LINE
#include "gui/gui_utils/graphics.h"

#include <functional>

#include <QColorDialog>    // DEBUG LINE
#include <QDebug>
#include <QInputDialog>    // DEBUG LINE

namespace hal
{
    NetlistRelay::NetlistRelay(QObject* parent) : QObject(parent), mModuleModel(new ModuleModel(this))
    {
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &NetlistRelay::debugHandleFileOpened);    // DEBUG LINE
        registerCallbacks();
        log_info("test", "register callbacks");
    }

    NetlistRelay::~NetlistRelay()
    {
        log_info("test", "unregister callbacks");
        netlist_event_handler::unregister_callback("relay");
        net_event_handler::unregister_callback("relay");
        gate_event_handler::unregister_callback("relay");
        module_event_handler::unregister_callback("relay");
    }

    void NetlistRelay::registerCallbacks()
    {
        netlist_event_handler::register_callback("relay",
                                                 std::function<void(netlist_event_handler::event, Netlist*, u32)>(
                                                     std::bind(&NetlistRelay::relayNetlistEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        net_event_handler::register_callback("relay",
                                             std::function<void(net_event_handler::event, Net*, u32)>(
                                                 std::bind(&NetlistRelay::relayNetEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gate_event_handler::register_callback("relay",
                                              std::function<void(gate_event_handler::event, Gate*, u32)>(
                                                  std::bind(&NetlistRelay::relayGateEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        module_event_handler::register_callback("relay",
                                                std::function<void(module_event_handler::event, Module*, u32)>(
                                                    std::bind(&NetlistRelay::relayModuleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        grouping_event_handler::register_callback("relay",
                                                std::function<void(grouping_event_handler::event, Grouping*, u32)>(
                                                    std::bind(&NetlistRelay::relayGroupingEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    }

    QColor NetlistRelay::getModuleColor(const u32 id)
    {
        return mModuleColors.value(id);
    }

    ModuleModel* NetlistRelay::getModuleModel()
    {
        return mModuleModel;
    }

    void NetlistRelay::debugChangeModuleName(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        bool ok;
        QString text = QInputDialog::getText(nullptr, "Rename Module", "New Name", QLineEdit::Normal, QString::fromStdString(m->get_name()), &ok);

        if (ok && !text.isEmpty())
            m->set_name(text.toStdString());
    }

    void NetlistRelay::debugChangeModuleType(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        bool ok;
        QString text = QInputDialog::getText(nullptr, "Change Module Type", "New Type", QLineEdit::Normal, QString::fromStdString(m->get_type()), &ok);

        if (ok && !text.isEmpty())
            m->set_type(text.toStdString());
    }

    void NetlistRelay::debugChangeModuleColor(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        QColor color = QColorDialog::getColor();

        if (!color.isValid())
            return;

        mModuleColors.insert(id, color);
        mModuleModel->updateModule(id);

        // Since color is our Overlay over the netlist data, no event is
        // automatically fired. We need to take care of that ourselves here.
        gGraphContextManager->handleModuleColorChanged(m);

        Q_EMIT moduleColorChanged(m);
    }

    void NetlistRelay::debugAddSelectionToModule(const u32 id)
    {
        // NOT THREADSAFE
        // DECIDE HOW TO HANDLE MODULES

        Module* m = gNetlist->get_module_by_id(id);

        assert(m);

        for (auto sel_id : gSelectionRelay->mSelectedGates)
        {
            Gate* g = gNetlist->get_gate_by_id(sel_id);

            if (g)
                m->assign_gate(g);
        }
    }

    void NetlistRelay::debugAddChildModule(const u32 id)
    {
        // NOT THREADSAFE

        bool ok;
        QString name = QInputDialog::getText(nullptr, "", "Module Name:", QLineEdit::Normal, "", &ok);

        if (!ok || name.isEmpty())
            return;

        Module* m = gNetlist->get_module_by_id(id);

        if (!m)
            return;

        gNetlist->create_module(gNetlist->get_unique_module_id(), name.toStdString(), m);
    }

    void NetlistRelay::debugDeleteModule(const u32 id)
    {
        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        gNetlist->delete_module(m);
    }

    void NetlistRelay::relayNetlistEvent(netlist_event_handler::event ev, Netlist* object, u32 associated_data)
    {
        if (!object)
            return;    // SHOULD NEVER BE REACHED

        //qDebug() << "relayNetlistEvent called: event ID =" << ev << "for object at" << object.get();
        //Q_EMIT netlistEvent(ev, object, associated_data);

        switch (ev)
        {
        case netlist_event_handler::event::id_changed:
        {
            ///< associated_data = old id

            Q_EMIT netlistIdChanged(object, associated_data);
            break;
        }
        case netlist_event_handler::event::input_filename_changed:
        {
            ///< no associated_data

            Q_EMIT netlistInputFilenameChanged(object);
            break;
        }
        case netlist_event_handler::event::design_name_changed:
        {
            ///< no associated_data

            Q_EMIT netlistDesignNameChanged(object);
            break;
        }
        case netlist_event_handler::event::device_name_changed:
        {
            ///< no associated_data

            Q_EMIT netlistDeviceNameChanged(object);
            break;
        }
        case netlist_event_handler::event::marked_global_vcc:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistMarkedGlobalVcc(object, associated_data);
            break;
        }
        case netlist_event_handler::event::marked_global_gnd:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistMarkedGlobalGnd(object, associated_data);
            break;
        }
        case netlist_event_handler::event::unmarked_global_vcc:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistUnmarkedGlobalVcc(object, associated_data);
            break;
        }
        case netlist_event_handler::event::unmarked_global_gnd:
        {
            ///< associated_data = id of gate

            Q_EMIT netlistUnmarkedGlobalGnd(object, associated_data);
            break;
        }
        case netlist_event_handler::event::marked_global_input:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleMarkedGlobalInput(associated_data);

            Q_EMIT netlistMarkedGlobalInput(object, associated_data);
            break;
        }
        case netlist_event_handler::event::marked_global_output:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleMarkedGlobalOutput(associated_data);

            Q_EMIT netlistMarkedGlobalOutput(object, associated_data);
            break;
        }
        case netlist_event_handler::event::marked_global_inout:
        {
            ///< associated_data = id of net

            Q_EMIT netlistMarkedGlobalInout(object, associated_data);
            break;
        }
        case netlist_event_handler::event::unmarked_global_input:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleUnmarkedGlobalInput(associated_data);

            Q_EMIT netlistUnmarkedGlobalInput(object, associated_data);
            break;
        }
        case netlist_event_handler::event::unmarked_global_output:
        {
            ///< associated_data = id of net
            gGraphContextManager->handleUnmarkedGlobalOutput(associated_data);

            Q_EMIT netlistUnmarkedGlobalOutput(object, associated_data);
            break;
        }
        case netlist_event_handler::event::unmarked_global_inout:
        {
            ///< associated_data = id of net

            Q_EMIT netlistUnmarkedGlobalInout(object, associated_data);
            break;
        }
        }
    }

    void NetlistRelay::relayGroupingEvent(grouping_event_handler::event ev, Grouping *object, u32 associated_data)
    {

        if (!object)
            return;    // SHOULD NEVER BE REACHED

        switch (ev)
        {
        case grouping_event_handler::event::created:
            Q_EMIT groupingCreated(object);
            break;
        case grouping_event_handler::event::removed:
            Q_EMIT groupingRemoved(object);
            break;
        case grouping_event_handler::name_changed:
            Q_EMIT groupingNameChanged(object);
            break;
        case grouping_event_handler::event::gate_assigned:
            Q_EMIT groupingGateAssigned(object,associated_data);
            break;
        case grouping_event_handler::event::gate_removed:
            Q_EMIT groupingGateRemoved(object,associated_data);
            break;
        case grouping_event_handler::event::net_assigned:
            Q_EMIT groupingNetAssigned(object,associated_data);
            break;
        case grouping_event_handler::event::net_removed:
            Q_EMIT groupingNetRemoved(object,associated_data);
            break;
        case grouping_event_handler::event::module_assigned:
            Q_EMIT groupingModuleAssigned(object,associated_data);
            break;
        case grouping_event_handler::event::module_removed:
            Q_EMIT groupingModuleRemoved(object,associated_data);
            break;
        }
    }

    void NetlistRelay::relayModuleEvent(module_event_handler::event ev, Module* object, u32 associated_data)
    {
        if (!object)
            return;    // SHOULD NEVER BE REACHED

        //qDebug() << "relayModuleEvent called: event ID =" << ev << "for object at" << object.get();
        //Q_EMIT moduleEvent(ev, object, associated_data);

        switch (ev)
        {
        case module_event_handler::event::created:
        {
            //< no associated_data

            // suppress actions if we receive this for the top module
            if (object->get_parent_module() != nullptr)
            {
                mModuleColors.insert(object->get_id(), gui_utility::getRandomColor());
            }

            Q_EMIT moduleCreated(object);
            break;
        }
        case module_event_handler::event::removed:
        {
            //< no associated_data

            mModuleColors.remove(object->get_id());

            gGraphContextManager->handleModuleRemoved(object);
            gSelectionRelay->handleModuleRemoved(object->get_id());

            Q_EMIT module_removed(object);
            break;
        }
        case module_event_handler::event::name_changed:
        {
            //< no associated_data

            mModuleModel->updateModule(object->get_id());

            gGraphContextManager->handleModuleNameChanged(object);

            Q_EMIT moduleNameChanged(object);
            break;
        }
        case module_event_handler::event::parent_changed:
        {
            //< no associated_data

            Q_EMIT moduleParentChanged(object);
            break;
        }
        case module_event_handler::event::submodule_added:
        {
            //< associated_data = id of added module

            mModuleModel->addModule(associated_data, object->get_id());

            gGraphContextManager->handleModuleSubmoduleAdded(object, associated_data);

            Q_EMIT moduleSubmoduleAdded(object, associated_data);
            break;
        }
        case module_event_handler::event::submodule_removed:
        {
            //< associated_data = id of removed module

            mModuleModel->remove_module(associated_data);

            gGraphContextManager->handleModuleSubmoduleRemoved(object, associated_data);

            Q_EMIT moduleSubmoduleRemoved(object, associated_data);
            break;
        }
        case module_event_handler::event::gate_assigned:
        {
            //< associated_data = id of inserted gate

            gGraphContextManager->handleModuleGateAssigned(object, associated_data);

            Q_EMIT moduleGateAssigned(object, associated_data);
            break;
        }
        case module_event_handler::event::gate_removed:
        {
            //< associated_data = id of removed gate

            gGraphContextManager->handleModuleGateRemoved(object, associated_data);

            Q_EMIT moduleGateRemoved(object, associated_data);
            break;
        }
        case module_event_handler::event::input_port_name_changed:
        {
            //< associated data = respective net

            gGraphContextManager->handleModuleInputPortNameChanged(object, associated_data);

            Q_EMIT moduleInputPortNameChanged(object, associated_data);
            break;
        }
        case module_event_handler::event::output_port_name_changed:
        {
            //< associated data = respective net

            gGraphContextManager->handleModuleOutputPortNameChanged(object, associated_data);

            Q_EMIT moduleOutputPortNameChanged(object, associated_data);
            break;
        }
        case module_event_handler::event::type_changed:
        {
            //< no associated_data

            gGraphContextManager->handleModuleTypeChanged(object);

            Q_EMIT moduleTypeChanged(object);
            break;
        }
        }
    }

    void NetlistRelay::relayGateEvent(gate_event_handler::event ev, Gate* object, u32 associated_data)
    {
        UNUSED(associated_data);
        if (!object)
            return;    // SHOULD NEVER BE REACHED

        //qDebug() << "relayGateEvent called: event ID =" << ev << "for object at" << object.get();
        //Q_EMIT gateEvent(ev, object, associated_data);

        switch (ev)
        {
        case gate_event_handler::event::created:
        {
            //< no associated_data

            Q_EMIT gateCreated(object);
            break;
        }
        case gate_event_handler::removed:
        {
            //< no associated_data

            gSelectionRelay->handleGateRemoved(object->get_id());

            Q_EMIT gate_removed(object);
            break;
        }
        case gate_event_handler::name_changed:
        {
            //< no associated_data

            gGraphContextManager->handleGateNameChanged(object);

            Q_EMIT gateNameChanged(object);
            break;
        }
        default:
            break;
        }
    }

    void NetlistRelay::relayNetEvent(net_event_handler::event ev, Net* object, u32 associated_data)
    {
        if (!object)
            return;    // SHOULD NEVER BE REACHED

        //qDebug() << "relayNetEvent called: event ID =" << ev << "for object at" << object.get();
        //Q_EMIT netEvent(ev, object, associated_data);

        switch (ev)
        {
        case net_event_handler::event::created:
        {
            //< no associated_data

            gGraphContextManager->handleNetCreated(object);

            Q_EMIT netCreated(object);
            break;
        }
        case net_event_handler::event::removed:
        {
            //< no associated_data

            gGraphContextManager->handleNetRemoved(object);
            gSelectionRelay->handleNetRemoved(object->get_id());

            Q_EMIT net_removed(object);
            break;
        }
        case net_event_handler::event::name_changed:
        {
            //< no associated_data

            gGraphContextManager->handleNetNameChanged(object);

            Q_EMIT netNameChanged(object);
            break;
        }
            // FIXME add src_added, src_removed
            // case net_event_handler::event::src_changed:
            // {
            //     //< no associated_data

            //     gGraphContextManager->handle_net_source_changed(object);

            //     Q_EMIT net_source_changed(object);
            //     break;
            // }
        case net_event_handler::event::src_added:
        {
            //< associated_data = id of src gate

            gGraphContextManager->handleNetSourceAdded(object, associated_data);

            Q_EMIT netSourceAdded(object, associated_data);
            break;
        }
        case net_event_handler::event::src_removed:
        {
            //< associated_data = id of src gate

            gGraphContextManager->handleNetSourceRemoved(object, associated_data);

            Q_EMIT netSourceRemoved(object, associated_data);
            break;
        }
        case net_event_handler::event::dst_added:
        {
            //< associated_data = id of dst gate

            gGraphContextManager->handleNetDestinationAdded(object, associated_data);

            Q_EMIT netDestinationAdded(object, associated_data);
            break;
        }
        case net_event_handler::event::dst_removed:
        {
            //< associated_data = id of dst gate

            gGraphContextManager->handleNetDestinationRemoved(object, associated_data);

            Q_EMIT netDestinationRemoved(object, associated_data);
            break;
        }
        }
    }

    void NetlistRelay::debugHandleFileOpened()
    {
        for (Module* m : gNetlist->get_modules())
            mModuleColors.insert(m->get_id(), gui_utility::getRandomColor());

        mModuleColors.insert(1, QColor(96, 110, 112));

        mModuleModel->init();
    }

    void NetlistRelay::debugHandleFileClosed()
    {
        mModuleModel->clear();
        mModuleColors.clear();
    }
}

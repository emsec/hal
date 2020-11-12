#include "gui/netlist_watcher/netlist_watcher.h"

#include "gui/gui_globals.h"

namespace hal
{
    NetlistWatcher::NetlistWatcher(QObject* parent) : QObject(parent)
    {
        netlist_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(netlist_event_handler::event, Netlist*, u32)>
                    (std::bind(&NetlistWatcher::handleNetlistEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        net_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(net_event_handler::event, Net*, u32)>
                    (std::bind(&NetlistWatcher::handleNetEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gate_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(gate_event_handler::event, Gate*, u32)>
                    (std::bind(&NetlistWatcher::handleGateEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        module_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(module_event_handler::event, Module*, u32)>
                    (std::bind(&NetlistWatcher::handleModuleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        reset();
    }

    NetlistWatcher::~NetlistWatcher()
    {
        netlist_event_handler::unregister_callback("NetlistWatcher");
        net_event_handler::unregister_callback("NetlistWatcher");
        gate_event_handler::unregister_callback("NetlistWatcher");
        module_event_handler::unregister_callback("NetlistWatcher");
    }

    void NetlistWatcher::handleNetlistEvent(netlist_event_handler::event ev, Netlist* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleModuleEvent(module_event_handler::event ev, Module* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleGateEvent(gate_event_handler::event ev, Gate* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleNetEvent(net_event_handler::event ev, Net* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::reset()
    {
        mNotified = false;
    }

    void NetlistWatcher::handleNetlistModified()
    {
        if (!mNotified)
        {
            mNotified = true;
            gFileStatusManager->netlistChanged();
        }
    }
}

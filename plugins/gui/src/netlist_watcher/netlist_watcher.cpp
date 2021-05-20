#include "gui/netlist_watcher/netlist_watcher.h"

#include "gui/gui_globals.h"

namespace hal
{
    NetlistWatcher::NetlistWatcher(QObject* parent) : QObject(parent)
    {
        gNetlist->get_event_handler()->register_callback(
                    "NetlistWatcher",
                    std::function<void(NetlistEvent::event, Netlist*, u32)>
                    (std::bind(&NetlistWatcher::handleNetlistEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
                    "NetWatcher",
                    std::function<void(NetEvent::event, Net*, u32)>
                    (std::bind(&NetlistWatcher::handleNetEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
                    "GateWatcher",
                    std::function<void(GateEvent::event, Gate*, u32)>
                    (std::bind(&NetlistWatcher::handleGateEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gNetlist->get_event_handler()->register_callback(
                    "ModuleWatcher",
                    std::function<void(ModuleEvent::event, Module*, u32)>
                    (std::bind(&NetlistWatcher::handleModuleEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        reset();
    }

    NetlistWatcher::~NetlistWatcher()
    {
        gNetlist->get_event_handler()->unregister_callback("NetlistWatcher");
        gNetlist->get_event_handler()->unregister_callback("ModuleWatcher");
        gNetlist->get_event_handler()->unregister_callback("GateWatcher");
        gNetlist->get_event_handler()->unregister_callback("NetWatcher");
    }

    void NetlistWatcher::handleNetlistEvent(NetlistEvent::event ev, Netlist *object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleModuleEvent(ModuleEvent::event ev, Module *mod, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(mod);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleGateEvent(GateEvent::event ev, Gate *gat, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(gat);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleNetEvent(NetEvent::event ev, Net *net, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(net);
        Q_UNUSED(associated_data);
        handleNetlistModified();
    }

    void NetlistWatcher::handleGroupingEvent(GroupingEvent::event ev, Grouping *grp, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(grp);
        Q_UNUSED(associated_data);
        // TODO: update grouping widget
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

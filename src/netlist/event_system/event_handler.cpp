#include "hal_core/netlist/event_system/event_handler.h"

#include "hal_core/netlist/event_system/event_log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    template<>
    std::map<NetlistEvent::event, std::string> EnumStrings<NetlistEvent::event>::data = {{NetlistEvent::event::id_changed, "id_changed"},
                                                                                         {NetlistEvent::event::input_filename_changed, "input_filename_changed"},
                                                                                         {NetlistEvent::event::design_name_changed, "design_name_changed"},
                                                                                         {NetlistEvent::event::device_name_changed, "device_name_changed"},
                                                                                         {NetlistEvent::event::marked_global_vcc, "marked_global_vcc"},
                                                                                         {NetlistEvent::event::marked_global_gnd, "marked_global_gnd"},
                                                                                         {NetlistEvent::event::unmarked_global_vcc, "unmarked_global_vcc"},
                                                                                         {NetlistEvent::event::unmarked_global_gnd, "unmarked_global_gnd"},
                                                                                         {NetlistEvent::event::marked_global_input, "marked_global_input"},
                                                                                         {NetlistEvent::event::marked_global_output, "marked_global_output"},
                                                                                         {NetlistEvent::event::unmarked_global_input, "unmarked_global_input"},
                                                                                         {NetlistEvent::event::unmarked_global_output, "unmarked_global_output"}};

    template<>
    std::map<GateEvent::event, std::string> EnumStrings<GateEvent::event>::data = {{GateEvent::event::created, "created"},
                                                                                   {GateEvent::event::removed, "removed"},
                                                                                   {GateEvent::event::name_changed, "name_changed"},
                                                                                   {GateEvent::event::location_changed, "location_changed"},
                                                                                   {GateEvent::event::boolean_function_changed, "boolean_function_changed"}};

    template<>
    std::map<NetEvent::event, std::string> EnumStrings<NetEvent::event>::data = {{NetEvent::event::created, "created"},
                                                                                 {NetEvent::event::removed, "removed"},
                                                                                 {NetEvent::event::name_changed, "name_changed"},
                                                                                 {NetEvent::event::src_added, "src_added"},
                                                                                 {NetEvent::event::src_removed, "src_removed"},
                                                                                 {NetEvent::event::dst_added, "dst_added"},
                                                                                 {NetEvent::event::dst_removed, "dst_removed"}};

    template<>
    std::map<ModuleEvent::event, std::string> EnumStrings<ModuleEvent::event>::data = {{ModuleEvent::event::created, "created"},
                                                                                       {ModuleEvent::event::removed, "removed"},
                                                                                       {ModuleEvent::event::name_changed, "name_changed"},
                                                                                       {ModuleEvent::event::type_changed, "type_changed"},
                                                                                       {ModuleEvent::event::parent_changed, "parent_changed"},
                                                                                       {ModuleEvent::event::submodule_added, "submodule_added"},
                                                                                       {ModuleEvent::event::submodule_removed, "submodule_removed"},
                                                                                       {ModuleEvent::event::gates_assign_begin, "gates_assign_begin"},
                                                                                       {ModuleEvent::event::gates_assign_end, "gates_assign_end"},
                                                                                       {ModuleEvent::event::gate_assigned, "gate_assigned"},
                                                                                       {ModuleEvent::event::gates_remove_begin, "gates_remove_begin"},
                                                                                       {ModuleEvent::event::gates_remove_end, "gates_remove_end"},
                                                                                       {ModuleEvent::event::gate_removed, "gate_removed"},
                                                                                       {ModuleEvent::event::pin_changed, "pin_changed"}};

    template<>
    std::map<GroupingEvent::event, std::string> EnumStrings<GroupingEvent::event>::data = {{GroupingEvent::event::created, "created"},
                                                                                           {GroupingEvent::event::removed, "removed"},
                                                                                           {GroupingEvent::event::name_changed, "name_changed"},
                                                                                           {GroupingEvent::event::gate_assigned, "gate_assigned"},
                                                                                           {GroupingEvent::event::gate_removed, "gate_removed"},
                                                                                           {GroupingEvent::event::net_assigned, "net_assigned"},
                                                                                           {GroupingEvent::event::net_removed, "net_removed"},
                                                                                           {GroupingEvent::event::module_assigned, "module_assigned"},
                                                                                           {GroupingEvent::event::module_removed, "module_removed"}};

    EventHandler::EventHandler() : netlist_event_enabled(true), module_event_enabled(true), gate_event_enabled(true), net_event_enabled(true), grouping_event_enabled(true)
    {
    }

    void EventHandler::event_enable_all(bool flag)
    {
        netlist_event_enabled  = flag;
        module_event_enabled   = flag;
        gate_event_enabled     = flag;
        net_event_enabled      = flag;
        grouping_event_enabled = flag;
    }

    void EventHandler::notify(NetlistEvent::event c, Netlist* netlist, u32 associated_data)
    {
        if (netlist_event_enabled)
        {
            m_netlist_callback(c, netlist, associated_data);
            event_log::handle_netlist_event(c, netlist, associated_data);
        }
    }

    void EventHandler::notify(GateEvent::event c, Gate* gate, u32 associated_data)
    {
        if (gate_event_enabled)
        {
            m_gate_callback(c, gate, associated_data);
            event_log::handle_gate_event(c, gate, associated_data);
        }
    }

    void EventHandler::notify(NetEvent::event c, Net* net, u32 associated_data)
    {
        if (net_event_enabled)
        {
            m_net_callback(c, net, associated_data);
            event_log::handle_net_event(c, net, associated_data);
        }
    }

    void EventHandler::notify(ModuleEvent::event c, Module* module, u32 associated_data)
    {
        //        ModuleEvent::dump(c, true);
        if (module_event_enabled)
        {
            m_module_callback(c, module, associated_data);
            event_log::handle_module_event(c, module, associated_data);
        }
    }

    void EventHandler::notify(GroupingEvent::event c, Grouping* grouping, u32 associated_data)
    {
        if (grouping_event_enabled)
        {
            m_grouping_callback(c, grouping, associated_data);
            event_log::handle_grouping_event(c, grouping, associated_data);
        }
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(GateEvent::event, Gate*, u32)> function)
    {
        m_gate_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(GroupingEvent::event, Grouping*, u32)> function)
    {
        m_grouping_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(ModuleEvent::event, Module*, u32)> function)
    {
        m_module_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(NetEvent::event, Net*, u32)> function)
    {
        m_net_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(NetlistEvent::event, Netlist*, u32)> function)
    {
        m_netlist_callback.add_callback(name, function);
    }

    void EventHandler::unregister_callback(const std::string& name)
    {
        m_netlist_callback.remove_callback(name);
        m_module_callback.remove_callback(name);
        m_gate_callback.remove_callback(name);
        m_net_callback.remove_callback(name);
        m_grouping_callback.remove_callback(name);
    }
}    // namespace hal

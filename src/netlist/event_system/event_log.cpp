#include "hal_core/netlist/event_system/event_log.h"

#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace event_log
    {

        namespace  {
            bool mEnableEventLog = false;
        }

        void enable_event_log(bool enable)
        {
            mEnableEventLog = enable;
        }

        void handle_gate_event(GateEvent::event event, Gate* gate, u32 associated_data)
            {
                UNUSED(associated_data);
                if (!mEnableEventLog) return;

                if (event == GateEvent::event::created)
                {
                    log_info("event", "created new gate '{}' (type '{}', id {:08x})", gate->get_name(), gate->get_type()->get_name(), gate->get_id());
                }
                else if (event == GateEvent::event::removed)
                {
                    log_info("event", "deleted gate '{}' (type '{}', id {:08x})", gate->get_name(), gate->get_type()->get_name(), gate->get_id());
                }
                else if (event == GateEvent::event::name_changed)
                {
                    log_info("event", "changed name of gate with id {:08x} to '{}'", gate->get_id(), gate->get_name());
                }
                else
                {
                    log_error("event", "unknown gate event");
                }
            }

            void handle_net_event(NetEvent::event event, Net* net, u32 associated_data)
            {
                if (!mEnableEventLog) return;

                if (event == NetEvent::event::created)
                {
                    log_info("event", "created new net '{}' (id {:08x})", net->get_name(), net->get_id());
                }
                else if (event == NetEvent::event::removed)
                {
                    log_info("event", "deleted net '{}' (id {:08x})", net->get_name(), net->get_id());
                }
                else if (event == NetEvent::event::name_changed)
                {
                    log_info("event", "changed name of net with id {:08x} to '{}'", net->get_id(), net->get_name());
                }
                else if (event == NetEvent::event::src_added)
                {
                    auto gate = net->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "added gate '{}' (id {:08x}) as a source for net '{}' (id {:08x})", gate->get_name(), gate->get_id(), net->get_name(), net->get_id());
                }
                else if (event == NetEvent::event::src_removed)
                {
                    auto gate = net->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "removed source gate '{}' (id {:08x}) from net '{}' (id {:08x})", gate->get_name(), gate->get_id(), net->get_name(), net->get_id());
                }
                else if (event == NetEvent::event::dst_added)
                {
                    auto gate = net->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "added gate '{}' (id {:08x}) as a destination for net '{}' (id {:08x})", gate->get_name(), gate->get_id(), net->get_name(), net->get_id());
                }
                else if (event == NetEvent::event::dst_removed)
                {
                    auto gate = net->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "removed destination gate '{}' (id {:08x}) from net '{}' (id {:08x})", gate->get_name(), gate->get_id(), net->get_name(), net->get_id());
                }
                else
                {
                    log_error("event", "unknown net event");
                }
            }

            void handle_netlist_event(NetlistEvent::event event, Netlist* netlist, u32 associated_data)
            {
                if (!mEnableEventLog) return;

                if (event == NetlistEvent::event::id_changed)
                {
                    log_info("event", "changed netlist id from {:08x} to {:08x}", associated_data, netlist->get_id());
                }
                else if (event == NetlistEvent::event::input_filename_changed)
                {
                    log_info("event", "changed input filename of netlist with id {:08x} to '{}'", netlist->get_id(), netlist->get_input_filename().string());
                }
                else if (event == NetlistEvent::event::design_name_changed)
                {
                    log_info("event", "changed design name of netlist with id {:08x} to '{}'", netlist->get_id(), netlist->get_design_name());
                }
                else if (event == NetlistEvent::event::device_name_changed)
                {
                    log_info("event", "changed target device name of netlist with id {:08x} to '{}'", netlist->get_id(), netlist->get_device_name());
                }
                else if (event == NetlistEvent::event::marked_global_vcc)
                {
                    auto gate = netlist->get_gate_by_id(associated_data);
                    log_info("event", "marked gate '{}' (id {:08x}) as a global vcc gate in netlist with id {:08x}", gate->get_name(), gate->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::marked_global_gnd)
                {
                    auto gate = netlist->get_gate_by_id(associated_data);
                    log_info("event", "marked gate '{}' (id {:08x}) as a global gnd gate in netlist with id {:08x}", gate->get_name(), gate->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::unmarked_global_vcc)
                {
                    auto gate = netlist->get_gate_by_id(associated_data);
                    log_info("event", "unmarked gate '{}' (id {:08x}) as a global vcc gate in netlist with id {:08x}", gate->get_name(), gate->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::unmarked_global_gnd)
                {
                    auto gate = netlist->get_gate_by_id(associated_data);
                    log_info("event", "unmarked gate '{}' (id {:08x}) as a global gnd gate in netlist with id {:08x}", gate->get_name(), gate->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::marked_global_input)
                {
                    auto net = netlist->get_net_by_id(associated_data);
                    log_info("event", "marked net '{}' (id {:08x}) as a global input net in netlist with id {:08x}", net->get_name(), net->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::marked_global_output)
                {
                    auto net = netlist->get_net_by_id(associated_data);
                    log_info("event", "marked net '{}' (id {:08x}) as a global output net in netlist with id {:08x}", net->get_name(), net->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::marked_global_inout)
                {
                    auto net = netlist->get_net_by_id(associated_data);
                    log_info("event", "marked net '{}' (id {:08x}) as a global inout net in netlist with id {:08x}", net->get_name(), net->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::unmarked_global_input)
                {
                    auto net = netlist->get_net_by_id(associated_data);
                    log_info("event", "unmarked net '{}' (id {:08x}) as a global input net in netlist with id {:08x}", net->get_name(), net->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::unmarked_global_output)
                {
                    auto net = netlist->get_net_by_id(associated_data);
                    log_info("event", "unmarked net '{}' (id {:08x}) as a global output net in netlist with id {:08x}", net->get_name(), net->get_id(), netlist->get_id());
                }
                else if (event == NetlistEvent::event::unmarked_global_inout)
                {
                    auto net = netlist->get_net_by_id(associated_data);
                    log_info("event", "unmarked net '{}' (id {:08x}) as a global inout net in netlist with id {:08x}", net->get_name(), net->get_id(), netlist->get_id());
                }
                else
                {
                    log_error("event", "unknown netlist event");
                }
            }

            void handle_grouping_event(GroupingEvent::event event, Grouping* grp, u32 associated_data)
            {
                if (!mEnableEventLog) return;

                if (event == GroupingEvent::event::created)
                {
                    log_info("event", "created new grouping '{}' (id {:08x})", grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::removed)
                {
                    log_info("event", "deleted grouping '{}' (id {:08x})", grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::name_changed)
                {
                    log_info("event", "changed name of net with id {:08x} to '{}'", grp->get_id(), grp->get_name());
                }
                else if (event == GroupingEvent::event::gate_assigned)
                {
                    Gate* gate = grp->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "added gate '{}' (id {:08x}) to grouping '{}' (id {:08x})", gate->get_name(), gate->get_id(), grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::gate_removed)
                {
                    Gate* gate = grp->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "removed gate '{}' (id {:08x}) from grouping '{}' (id {:08x})", gate->get_name(), gate->get_id(), grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::net_assigned)
                {
                    Net* net = grp->get_netlist()->get_net_by_id(associated_data);
                    log_info("event", "added net '{}' (id {:08x}) to grouping '{}' (id {:08x})", net->get_name(), net->get_id(), grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::net_removed)
                {
                    Net* net = grp->get_netlist()->get_net_by_id(associated_data);
                    log_info("event", "removed net '{}' (id {:08x}) from grouping '{}' (id {:08x})", net->get_name(), net->get_id(), grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::module_assigned)
                {
                    Module* m = grp->get_netlist()->get_module_by_id(associated_data);
                    log_info("event", "added module '{}' (id {:08x}) to grouping '{}' (id {:08x})", m->get_name(), m->get_id(), grp->get_name(), grp->get_id());
                }
                else if (event == GroupingEvent::event::module_removed)
                {
                    Module* m = grp->get_netlist()->get_module_by_id(associated_data);
                    log_info("event", "removed module '{}' (id {:08x}) from grouping '{}' (id {:08x})", m->get_name(), m->get_id(), grp->get_name(), grp->get_id());
                }
                else
                {
                    log_error("event", "unknown grouping event");
                }
            }

            void handle_module_event(ModuleEvent::event event, Module* module, u32 associated_data)
            {
                if (!mEnableEventLog) return;

                if (event == ModuleEvent::event::created)
                {
                    log_info("event", "created new submodule '{}' (id {:08x})", module->get_name(), module->get_id());
                }
                else if (event == ModuleEvent::event::removed)
                {
                    log_info("event", "deleted submodule '{}' (id {:08x})", module->get_name(), module->get_id());
                }
                else if (event == ModuleEvent::event::name_changed)
                {
                    log_info("event", "changed name of module '{}' (id {:08x}) to '{}'", module->get_name(), module->get_id(), module->get_name());
                }
                else if (event == ModuleEvent::event::type_changed)
                {
                    log_info("event", "changed type of module '{}' (id {:08x}) to '{}'", module->get_name(), module->get_id(), module->get_type());
                }
                else if (event == ModuleEvent::event::parent_changed)
                {
                    log_info("event",
                             "changed parent of submodule '{}' (id {:08x}) to module '{}' (id {:08x})",
                             module->get_name(),
                             module->get_id(),
                             module->get_parent_module()->get_name(),
                             module->get_parent_module()->get_id());
                }
                else if (event == ModuleEvent::event::submodule_added)
                {
                    log_info("event",
                             "added submodule '{}' (id {:08x}) to module '{}' (id {:08x})",
                             module->get_netlist()->get_module_by_id(associated_data)->get_name(),
                             associated_data,
                             module->get_name(),
                             module->get_id());
                }
                else if (event == ModuleEvent::event::submodule_removed)
                {
                    log_info("event", "removed submodule with id {:08x} from module '{}' (id {:08x})", associated_data, module->get_name(), module->get_id());
                }
                else if (event == ModuleEvent::event::gate_assigned)
                {
                    auto gate = module->get_netlist()->get_gate_by_id(associated_data);
                    log_info("event", "inserted gate '{}' (id {:08x}) into module '{}' (id {:08x})", gate->get_name(), associated_data, module->get_name(), module->get_id());
                }
                else if (event == ModuleEvent::event::gate_removed)
                {
                    log_info("event", "removed gate with id {:08x} from module '{}' (id {:08x})", associated_data, module->get_name(), module->get_id());
                }
                else if (event == ModuleEvent::event::input_port_name_changed)
                {
                    log_info("event", "changed input port name of net with id {:08x} from module '{}' (id {:08x})", associated_data, module->get_name(), module->get_id());
                }
                else if (event == ModuleEvent::event::output_port_name_changed)
                {
                    log_info("event", "changed output port name of net with id {:08x} from module '{}' (id {:08x})", associated_data, module->get_name(), module->get_id());
                }
                else
                {
                    log_error("event", "unknown module event");
                }
            }
    }    // namespace event_log

    void initialize()
    {
        LogManager::get_instance().add_channel("event", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

            
}    // namespace hal

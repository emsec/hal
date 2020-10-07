#pragma once

#include "hal_core/netlist/netlist_internal_manager.h"

#include <unordered_map>
#include <vector>

namespace hal
{
    class Gate;
    class Net;
    class Module;

    class NETLIST_API Grouping
    {
    public:
        u32 get_id();
        void set_name(std::string name);
        std::string get_name();

        bool assign_gate(Gate* gate);
        bool assign_gate_by_id(const u32 gate_id);
        std::vector<Gate*> get_gates();
        std::vector<u32> get_gate_ids();
        bool remove_gate(Gate* gate);
        bool remove_gate_by_id(const u32 gate_id);
        bool contains_gate(Gate* gate);
        bool contains_gate_by_id(const u32 gate_id);

        bool assign_net(Net* net);
        bool assign_net_by_id(const u32 net_id);
        std::vector<Net*> get_nets();
        std::vector<u32> get_net_ids();
        bool remove_net(Net* net);
        bool remove_net_by_id(const u32 net_id);
        bool contains_net(Net* net);
        bool contains_net_by_id(const u32 net_id);

        bool assign_module(Module* module);
        bool assign_module_by_id(const u32 module_id);
        std::vector<Module*> get_modules();
        std::vector<u32> get_module_ids();
        bool remove_module(Module* module);
        bool remove_module_by_id(const u32 module_id);
        bool contains_module(Module* module);
        bool contains_module_by_id(const u32 module_id);

    private:
        friend class NetlistInternalManager;

        Grouping(NetlistInternalManager* internal_manager, u32 id, std::string name);

        u32 m_id;
        std::string m_name;
        NetlistInternalManager* m_internal_manager;

        std::vector<Gate*> m_gates;
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Net*> m_nets;
        std::unordered_map<u32, Net*> m_nets_map;
        std::vector<Module*> m_modules;
        std::unordered_map<u32, Module*> m_modules_map;
    };
}    // namespace hal
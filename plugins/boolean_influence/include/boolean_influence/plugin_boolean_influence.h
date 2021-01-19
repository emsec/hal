#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

#include "hal_core/netlist/gate.h"

namespace hal
{
    class PLUGIN_API BooleanInfluencePlugin : public BasePluginInterface
    {
    public:

        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        std::map<Gate*, double> get_boolean_influences_of_gate(const Gate* gate);

    private:
        std::vector<Gate*> extract_function_gates(const Gate* start, const std::string& pin);
        void add_inputs(Gate* gate, std::unordered_set<Gate*>& gates);
    };
}

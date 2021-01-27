#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

#include "hal_core/netlist/net.h"
#include "hal_core/netlist/gate.h"

namespace hal
{
    class PLUGIN_API BooleanInfluencePlugin : public BasePluginInterface
    {
    public:

        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        /**
         * Generates the function of the dataport net of the given flip-flop.
         * Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evalated.G
         *
         * @param[in] gate - Pointer to the flip-flop which data input net is used to build the boolean function.
         * @returns A mapping of the gates that appear in the function of the data net to their boolean influence in said function.
         */
        std::map<Net*, double> get_boolean_influences_of_gate(const Gate* gate);

    private:
        std::vector<Gate*> extract_function_gates(const Gate* start, const std::string& pin);
        void add_inputs(Gate* gate, std::unordered_set<Gate*>& gates);
    };
}

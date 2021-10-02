#include "netlist_simulator_controller/simulation_input.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal {

    bool SimulationInput::contains_gate(const Gate* g) const
    {
        return (mSimulationSet.find(g) != mSimulationSet.end());
    }

    const std::unordered_set<const Gate*>& SimulationInput::gates() const
    {
        return mSimulationSet;
    }

    void SimulationInput::add_gates(const std::vector<Gate*>& gates)
    {
        mSimulationSet.insert(gates.begin(), gates.end());

        compute_input_nets();
        compute_output_nets();
    }



}

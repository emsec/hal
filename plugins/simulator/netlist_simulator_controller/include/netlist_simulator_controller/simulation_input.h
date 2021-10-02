#pragma once

#include <vector>
#include <unordered_set>
#include "hal_core/defines.h"

namespace hal {
    class Gate;
    class Net;

    class SimulationInput {

        struct Clock
        {
            const Net* clock_net;
            u64 switch_time;
            bool start_at_zero;
        };

        std::unordered_set<const Gate*> mSimulationSet;
        std::vector<Clock> m_clocks;

        std::vector<const Net*> m_input_nets;
        std::vector<const Net*> m_output_nets;

    public:
        SimulationInput() {;}
        bool contains_gate(const Gate* g) const;
        const std::unordered_set<const Gate *>& gates() const;

        void compute_input_nets();
        void compute_output_nets();

        /**
         * Add gates to the simulation set that contains all gates that are considered during simulation.
         * This function can only be called before the simulation has been initialized.
         *
         * @param[in] gates - The gates to add.
         */
        void add_gates(const std::vector<Gate*>& gates);
    };
}

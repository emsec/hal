#pragma once

#include "hal_core/defines.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Gate;
    class Net;

    namespace gate_library_specific_utils
    {
        class Utils
        {
        public:
            Utils()          = default;
            virtual ~Utils() = default;

            /* library specific functions */
            virtual bool is_sequential(Gate* sg) const                                          = 0;
            virtual std::unordered_set<std::string> get_control_input_pin_types(Gate* sg) const = 0;
            virtual std::unordered_set<std::string> get_clock_ports(Gate* sg) const             = 0;
            virtual std::unordered_set<std::string> get_enable_ports(Gate* sg) const            = 0;
            virtual std::unordered_set<std::string> get_reset_ports(Gate* sg) const             = 0;
            virtual std::unordered_set<std::string> get_set_ports(Gate* sg) const               = 0;
            virtual std::unordered_set<std::string> get_data_ports(Gate* sg) const              = 0;

            virtual std::unordered_set<std::string> get_regular_outputs(Gate* sg) const         = 0;
            virtual std::unordered_set<std::string> get_negated_outputs(Gate* sg) const         = 0;

            std::unordered_set<Gate*> get_sequential_successors(Gate* start_gate);
            std::unordered_set<Gate*> get_sequential_successors(Net* start_net);
            void clear_successor_cache();

            std::unordered_set<Net*> get_clock_signals_of_gate(Gate* sg);
            std::unordered_set<Net*> get_enable_signals_of_gate(Gate* sg);
            std::unordered_set<Net*> get_reset_signals_of_gate(Gate* sg);
            std::unordered_set<Net*> get_set_signals_of_gate(Gate* sg);
            std::unordered_set<Net*> get_data_signals_of_gate(Gate* sg);

        private:
            std::unordered_map<u32, std::unordered_set<Gate*>> m_successor_cache;
            std::unordered_set<Gate*> get_sequential_successors_internal(Net* start_net, std::unordered_set<u32>& seen);
        };
    }   // namespace gate_library_specific_utils
}    // namespace hal
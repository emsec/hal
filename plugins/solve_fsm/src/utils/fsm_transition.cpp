#include "utils/fsm_transition.h"

namespace hal {
    FsmTransition::FsmTransition(const z3::expr& start, const z3::expr& end, const std::map<u32, u8>& inputs) : starting_state_expr(start), end_state_expr(end) {
        if (!start.is_numeral()) {
            log_error("FSM Solver", "starting state is not a numeral.");
        }
        if (!end.is_numeral()) {
            log_error("FSM Solver", "end state is not a numeral.");
        }

        starting_state = start.get_numeral_uint64();
        end_state = end.get_numeral_uint64();

        if (!inputs.empty()) {
            input_ids_to_values = {inputs};
        }
    }

    // FsmTransition::FsmTransition(const u64& start, const u64& end, const std::map<u32, u8>& inputs) : starting_state(start), end_state(end) {
    //     if (!inputs.empty()) {
    //         input_ids_to_values = {inputs};
    //     }
    // }

    FsmTransition FsmTransition::merge(const FsmTransition& other) const {
        if (starting_state != other.starting_state) {
            log_error("FSM Solver", "cant merge tansitions that do not start in the same state. ({} vs. {})", starting_state, other.starting_state);
        }
        if (end_state != other.end_state) {
            log_error("FSM Solver", "cant merge tansitions that do not end in the same state. ({} vs. {})", end_state, other.end_state);
        }

        FsmTransition n = other;
        for (const auto& m : input_ids_to_values) {
            n.input_ids_to_values.push_back(m);
        }

        // Check wether all possible conditions are covered and therefore the transition is always taken -> this is not exhaustive
        u32 max_inputs = 0;
        for (const auto& mapping : n.input_ids_to_values) {
            if (mapping.size() > max_inputs) {
                max_inputs = mapping.size();
            }
        }

        // all possible conditions are covered -> we can omit them.
        if (n.input_ids_to_values.size() == (u64(1) << max_inputs)) {
            n.input_ids_to_values = {};
        }

        return n;
    }

    std::string FsmTransition::to_string() const {
        if (input_ids_to_values.empty()) {
            return std::to_string(starting_state) + " -> " + std::to_string(end_state);
        }

        std::string input_str = "(";
        for (const auto& mapping : input_ids_to_values) {
            input_str += "[";
            for (const auto& [id, val] : mapping) {
                input_str += "{" + std::to_string(id) + ": " + std::to_string(val) + "}, ";
            }
            input_str += "]";
        }
        input_str += ")";

        return std::to_string(starting_state) + " -> " + std::to_string(end_state) + " " + input_str;
    }

    std::string FsmTransition::to_dot_string(const Netlist* nl) const {
        if (input_ids_to_values.empty()) {
            return std::to_string(starting_state) + " -> " + std::to_string(end_state) + ";\n";
        }
        
        const u32 size = input_ids_to_values.size();
        
        // Annotate Input order:
        std::string input_str = "[";
        for (const auto& [id, _] : input_ids_to_values.front()) {
            auto name = nl->get_net_by_id(id)->get_name();
            input_str += name + ", ";
        }
        input_str = input_str.erase(input_str.size()-2, 2);
        input_str += "] [" + std::to_string(size) + "]";

        // Dont show inputs if there are too many
        if (size > 8) {
            input_str += "...";
            return std::to_string(starting_state) + " -> " + std::to_string(end_state) + "[label=\"" + input_str + "\", weight=\"" + input_str + "\"];\n";
        }

        // print all inputs
        input_str += "(";
        for (const auto& mapping : input_ids_to_values) {
            for (const auto& [_, val] : mapping) {
                input_str += std::to_string(val);
            }
            input_str += ", ";
        }
        input_str = input_str.erase(input_str.size()-2, 2);
        input_str += ")";

        return std::to_string(starting_state) + " -> " + std::to_string(end_state) + "[label=\"" + input_str + "\", weight=\"" + input_str + "\"];\n";
    }
}  // namespace hal
#include "hal_core/netlist/boolean_function/symbolic_state.h"

namespace hal
{
    namespace SMT
    {
        SymbolicState::SymbolicState(const std::vector<BooleanFunction>& variables)
        {
            for (const auto& v : variables)
            {
                if (v.is_variable())
                {
                    this->variable.emplace(v.clone(), v.clone());
                }
            }
        }

        const BooleanFunction& SymbolicState::get(const BooleanFunction& key) const
        {
            auto it = this->variable.find(key);
            return (it == this->variable.end()) ? key : it->second;
        }

        std::unordered_map<std::string, const BooleanFunction*> SymbolicState::get_bindings() const
        {
            std::unordered_map<std::string, const BooleanFunction*> res;
            for (const auto& [key, value] : this->variable)
            {
                const auto& key_node = key.get_top_level_node();
                if (key_node.is_variable())
                {
                    res.emplace(key_node.variable, &value);
                }
            }
            return res;
        }

        void SymbolicState::set(const BooleanFunction& key, const BooleanFunction& value)
        {
            if (key.is_variable())
            {
                // insert_or_assign, not emplace: emplace leaves an existing binding untouched, so
                // setting a variable a second time did nothing and a loop that steps a state forward
                // silently kept the value it started with.
                this->variable.insert_or_assign(key.clone(), value.clone());
            }
        }
    }    // namespace SMT
}    // namespace hal
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

        void SymbolicState::set(const BooleanFunction& key, const BooleanFunction& value)
        {
            if (key.is_variable())
            {
                this->variable.emplace(std::move(key), std::move(value));
            }
        }
    }    // namespace SMT
}    // namespace hal
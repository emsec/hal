#include "hal_core/netlist/boolean_function/simplification.h"
#include "hal_core/netlist/boolean_function/symbolic_execution.h"

namespace hal
{
    Result<BooleanFunction> Simplification::local_simplification(const BooleanFunction& function)
    {
        auto current = function.clone(), before = BooleanFunction();

        do
        {
            before          = current.clone();
            auto simplified = SMT::SymbolicExecution().evaluate(current);
            if (simplified.is_error())
            {
                return ERR_APPEND(simplified.get_error(), "could not apply local simplification: symbolic execution failed");
            }
            current = simplified.get();
        } while (before != current);

        return OK(current);
    }
}    // namespace hal
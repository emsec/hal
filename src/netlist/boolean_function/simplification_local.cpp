#include "hal_core/netlist/boolean_function/simplification.h"

#include "hal_core/netlist/boolean_function/symbolic_execution.h"

namespace hal 
{
    Result<BooleanFunction> Simplification::local_simplification(const BooleanFunction& function)
    {
       	auto current = function.clone(),
             before = BooleanFunction();

        do
        {
            before          = current.clone();
            auto simplified = SMT::SymbolicExecution().evaluate(current);
            if (std::get_if<std::string>(&simplified) != nullptr)
            {
                return ERR(std::get<std::string>(simplified));
            }
            current = std::get<BooleanFunction>(simplified);
        } while (before != current);

        return OK(current);
    }
}  // namespace hal
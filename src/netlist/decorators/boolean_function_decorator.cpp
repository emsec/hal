#include "hal_core/netlist/decorators/boolean_function_decorator.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    BooleanFunctionDecorator::BooleanFunctionDecorator(const BooleanFunction& bf) : m_bf(bf)
    {
    }

    Result<BooleanFunction> BooleanFunctionDecorator::substitute_power_ground_nets(const Netlist* nl) const
    {
        BooleanFunction tmp_bf = m_bf.clone();

        for (const std::string& var_name : m_bf.get_variable_names())
        {
            const auto net_res = BooleanFunctionNetDecorator::get_net_from(nl, var_name);
            if (net_res.is_error())
            {
                return ERR(net_res.get_error());
            }
            const Net* net = net_res.get();

            if (const auto sources = net->get_sources(); sources.size() == 1)
            {
                if (sources.front()->get_gate()->get_type()->has_property(GateTypeProperty::power))
                {
                    if (auto subs_1_res = tmp_bf.substitute(var_name, BooleanFunction::Const(BooleanFunction::Value::ONE)); subs_1_res.is_ok())
                    {
                        tmp_bf = subs_1_res.get();
                    }
                    else
                    {
                        return ERR(subs_1_res.get_error());
                    }
                }
                else if (sources.front()->get_gate()->get_type()->has_property(GateTypeProperty::ground))
                {
                    if (auto subs_0_res = tmp_bf.substitute(var_name, BooleanFunction::Const(BooleanFunction::Value::ZERO)); subs_0_res.is_ok())
                    {
                        tmp_bf = subs_0_res.get();
                    }
                    else
                    {
                        return ERR(subs_0_res.get_error());
                    }
                }
            }
        }
        return OK(tmp_bf);
    }

    Result<BooleanFunction> get_boolean_function_from(const std::vector<Net*>& nets, u32 extend_to_size, bool sign_extend)
    {
        if (nets.empty())
        {
            return ERR("could not concatenate nets: no nets provided.");
        }

        if (nets.front() == nullptr)
        {
            return ERR("could not concatenate nets: nets contain a 'nullptr'.");
        }
        auto var = BooleanFunction::Var(BooleanFunctionNetDecorator(*(nets.front())).get_boolean_variable_name(), 1);

        for (u32 i = 1; i < nets.size(); i++)
        {
            if (nets.at(i) == nullptr)
            {
                return ERR("could not concatenate nets: nets contain a 'nullptr'.");
            }

            if (auto res = BooleanFunction::Concat(BooleanFunction::Var(BooleanFunctionNetDecorator(*(nets.at(i))).get_boolean_variable_name(), 1), var.clone(), i + 1); res.is_error())
            {
                var = res.get();
            }
            else
            {
                return ERR_APPEND(res.get_error(),
                                  "could not concatenate nets: unable to concatenate net '" + nets.at(i)->get_name() + "' with ID " + std::to_string(nets.at(i)->get_id()) + " at position "
                                      + std::to_string(i) + " of the provided nets to function.");
            }
        }

        if (extend_to_size > 0)
        {
            if (sign_extend == false)
            {
                if (auto res = BooleanFunction::Zext(var.clone(), BooleanFunction::Index(extend_to_size, extend_to_size), extend_to_size); res.is_error())
                {
                    var = res.get();
                }
                else
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not concatenate nets: unable to zero extend function of size " + std::to_string(var.size()) + " to size " + std::to_string(extend_to_size) + ".");
                }
            }
            else
            {
                if (auto res = BooleanFunction::Sext(var.clone(), BooleanFunction::Index(extend_to_size, extend_to_size), extend_to_size); res.is_error())
                {
                    var = res.get();
                }
                else
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not concatenate nets: unable to sign extend function of size " + std::to_string(var.size()) + " to size " + std::to_string(extend_to_size) + ".");
                }
            }
        }

        return OK(var);
    }
}    // namespace hal
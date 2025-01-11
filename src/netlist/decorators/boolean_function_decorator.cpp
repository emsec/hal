#include "hal_core/netlist/decorators/boolean_function_decorator.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
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

    // TODO think about moving this to its own BooleanFunctionGate Decorator
    Result<BooleanFunction> BooleanFunctionDecorator::substitute_power_ground_pins(const Gate* g) const
    {
        BooleanFunction tmp_bf = m_bf.clone();

        for (const std::string& var_name : m_bf.get_variable_names())
        {
            const Net* net = g->get_fan_in_net(var_name);
            if (net == nullptr)
            {
                return ERR("Unable to replace pins connected to GND/VCC with constants for Boolean function associated with gate " + g->get_name() + " with ID " + std::to_string(g->get_id())
                           + ": failed to find net connected to pin " + var_name);
            }

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

    Result<BooleanFunction> BooleanFunctionDecorator::substitute_module_pins(const std::vector<Module*>& modules) const
    {
        if (modules.empty())
        {
            return OK(m_bf);
        }

        const auto& netlist = modules.front()->get_netlist();
        auto tmp_bf         = m_bf.clone();

        for (const auto& var : m_bf.get_variable_names())
        {
            const auto var_net_res = BooleanFunctionNetDecorator::get_net_from(netlist, var);
            if (var_net_res.is_error())
            {
                continue;
            }
            const auto var_net = var_net_res.get();

            for (const auto& m : modules)
            {
                const auto& pin = m->get_pin_by_net(var_net);
                if (pin == nullptr)
                {
                    continue;
                }

                const auto& [pg, index] = pin->get_group();

                if (index > pg->size())
                {
                    return ERR("cannot substitute variables with module pins: pin " + pin->get_name() + " has index " + std::to_string(index) + " with is larger than the size of its pin group "
                               + pg->get_name());
                }

                const auto var_name = m->get_name() + "_" + pg->get_name();
                const auto index_bf = BooleanFunction::Index(index, pg->size());
                const auto sub_bf   = BooleanFunction::Slice(BooleanFunction::Var(var_name, pg->size()), index_bf.clone(), index_bf.clone(), 1).get();
                tmp_bf              = tmp_bf.substitute(var, sub_bf).get();

                // only consider first found module pin
                break;
            }
        }

        return OK(tmp_bf);
    }

    Result<BooleanFunction> BooleanFunctionDecorator::get_boolean_function_from(const std::vector<BooleanFunction>& functions, u32 extend_to_size, bool sign_extend)
    {
        if (functions.empty())
        {
            return ERR("could not concatenate functions: no functions provided.");
        }

        auto var = functions.front().clone();
        u32 size = var.size();

        for (u32 i = 1; i < functions.size(); i++)
        {
            size += functions.at(i).size();
            if (auto res = BooleanFunction::Concat(var.clone(), functions.at(i).clone(), size); res.is_ok())
            {
                var = res.get();
            }
            else
            {
                return ERR_APPEND(res.get_error(),
                                  "could not concatenate nets: unable to concatenate Boolean function '" + functions.at(i).to_string() + " at position " + std::to_string(i)
                                      + " of the provided functions to function.");
            }
        }

        if (extend_to_size > 0)
        {
            if (sign_extend == false)
            {
                if (auto res = BooleanFunction::Zext(var.clone(), BooleanFunction::Index(extend_to_size, extend_to_size), extend_to_size); res.is_ok())
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
                if (auto res = BooleanFunction::Sext(var.clone(), BooleanFunction::Index(extend_to_size, extend_to_size), extend_to_size); res.is_ok())
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

    Result<BooleanFunction> BooleanFunctionDecorator::get_boolean_function_from(const std::vector<Net*>& nets, u32 extend_to_size, bool sign_extend)
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

            if (auto res = BooleanFunction::Concat(var.clone(), BooleanFunction::Var(BooleanFunctionNetDecorator(*(nets.at(i))).get_boolean_variable_name(), 1), i + 1); res.is_ok())
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
                if (auto res = BooleanFunction::Zext(var.clone(), BooleanFunction::Index(extend_to_size, extend_to_size), extend_to_size); res.is_ok())
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
                if (auto res = BooleanFunction::Sext(var.clone(), BooleanFunction::Index(extend_to_size, extend_to_size), extend_to_size); res.is_ok())
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

    Result<BooleanFunction> BooleanFunctionDecorator::get_boolean_function_from(const PinGroup<ModulePin>* pin_group, u32 extend_to_size, bool sign_extend)
    {
        const auto pins = pin_group->get_pins();
        std::vector<BooleanFunction> functions;
        std::transform(pins.rbegin(), pins.rend(), std::back_inserter(functions), [](const ModulePin* pin) {
            return BooleanFunction::Var(BooleanFunctionNetDecorator(*(pin->get_net())).get_boolean_variable_name(), 1);
        });

        return get_boolean_function_from(functions, extend_to_size, sign_extend);
    }
}    // namespace hal
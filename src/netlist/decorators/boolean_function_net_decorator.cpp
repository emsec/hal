#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"

#include "hal_core/netlist/netlist.h"

namespace hal
{
    const std::string BooleanFunctionNetDecorator::VAR_NET_PREFIX = "net_";

    BooleanFunctionNetDecorator::BooleanFunctionNetDecorator(const Net& net) : m_net(net)
    {
    }

    BooleanFunction BooleanFunctionNetDecorator::get_boolean_variable() const
    {
        return BooleanFunction::Var(get_boolean_variable_name());
    }

    std::string BooleanFunctionNetDecorator::get_boolean_variable_name() const
    {
        return "net_" + std::to_string(m_net.get_id());
    }

    Result<Net*> BooleanFunctionNetDecorator::get_net_from(const Netlist* netlist, const std::string& var_name)
    {
        if (var_name.rfind(VAR_NET_PREFIX, 0) != 0)
        {
            return ERR("could not get net from string '" + var_name + "': string does not contain '" + VAR_NET_PREFIX + "' prefix");
        }

        try
        {
            return OK(netlist->get_net_by_id(std::stoul(var_name.substr(VAR_NET_PREFIX.size()))));
        }
        catch (const std::invalid_argument& e)
        {
            return ERR("could not get net from string '" + var_name + "': " + e.what());
        }
        catch (const std::out_of_range& e)
        {
            return ERR("could not get net from string '" + var_name + "': " + e.what());
        }
    }

    Result<Net*> BooleanFunctionNetDecorator::get_net_from(const Netlist* netlist, const BooleanFunction& var)
    {
        if (const auto& var_name_res = var.get_variable_name(); var_name_res.is_ok())
        {
            if (const auto& net_res = get_net_from(netlist, var_name_res.get()); net_res.is_ok())
            {
                return net_res;
            }
            else
            {
                return ERR_APPEND(net_res.get_error(), "could not get net from Boolean function '" + var.to_string() + "': unable to get net from variable name");
            }
        }
        else
        {
            return ERR_APPEND(var_name_res.get_error(), "could not get net from Boolean function '" + var.to_string() + "': unable to get variable name");
        }
    }
}    // namespace hal
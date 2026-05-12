#include "hal_core/netlist/data_container.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    bool DataContainer::operator==(const DataContainer& other) const
    {
        return m_data == other.get_data_map() && m_parameters == other.get_parameters();
    }

    bool DataContainer::operator!=(const DataContainer& other) const
    {
        return !operator==(other);
    }

    bool DataContainer::set_data(const std::string& category, const std::string& key, const std::string& value_data_type, const std::string& value, const bool log_with_info_level)
    {
        if (category.empty() || key.empty())
        {
            log_error("netlist", "key category or key is empty.");
            return false;
        }

        m_data[std::make_tuple(category, key)] = std::make_tuple(value_data_type, value);

        //notify_updated();

        if (log_with_info_level)
        {
            log_info("netlist", "added {} data '{}' ({}, {}).", value_data_type, value, category, key);
        }
        else
        {
            log_debug("netlist", "added {} data '{}' ({}, {}).", value_data_type, value, category, key);
        }

        return true;
    }

    bool DataContainer::delete_data(const std::string& category, const std::string& key, const bool log_with_info_level)
    {
        if (category.empty() || key.empty())
        {
            log_error("netlist", "key category or key is empty.");
            return false;
        }

        auto it = m_data.find(std::make_tuple(category, key));
        if (it == m_data.end())
        {
            log_debug("netlist", "no key ('{}', '{}') found.", category, key);
            return true;
        }

        auto deleted_value = std::get<1>(it->second);
        m_data.erase(it);

        //notify_updated();

        if (log_with_info_level)
        {
            log_info("netlist", "removed data '{}' ({}, {}).", deleted_value, category, key);
        }
        else
        {
            log_debug("netlist", "removed data '{}' ({}, {}).", deleted_value, category, key);
        }
        return true;
    }

    const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& DataContainer::get_data_map() const
    {
        return m_data;
    }

    void DataContainer::set_data_map(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& map)
    {
        m_data = map;
    }

    bool DataContainer::has_data(const std::string& category, const std::string& key) const
    {
        if (category.empty() || key.empty())
        {
            return false;
        }

        if (auto it = m_data.find(std::make_tuple(category, key)); it == m_data.end())
        {
            return false;
        }

        return true;
    }

    std::tuple<std::string, std::string> DataContainer::get_data(const std::string& category, const std::string& key) const
    {
        if (category.empty() || key.empty())
        {
            log_error("netlist", "key category or key is empty.");
            return std::make_tuple("", "");
        }

        auto it = m_data.find(std::make_tuple(category, key));
        if (it == m_data.end())
        {
            log_debug("netlist", "no value stored for key ('{}', '{}').", category, key);
            return std::make_tuple("", "");
        }
        return it->second;
    }

    Result<std::monostate> DataContainer::set_parameter(const Parameter& param, const std::string& value)
    {
        if (m_parameters.find(param.name) != m_parameters.end())
        {
            return ERR("could not set parameter '" + param.name + ": a parameter with that name already exists");
        }

        if (!param.validate(value))
        {
            return ERR("invalid parameter value");
        }

        m_parameters.insert_or_assign(param.name, std::make_pair(param, value));

        return OK({});
    }

    Result<std::string> DataContainer::get_parameter_value(const std::string& name) const
    {
        if (auto it = m_parameters.find(name); it != m_parameters.end())
        {
            return OK(it->second.second);
        }

        return ERR("no parameter named '" + name + "'");
    }

    Result<std::string> DataContainer::get_parameter_value(const Parameter& param) const
    {
        auto it = m_parameters.find(param.name);
        if (it == m_parameters.end())
        {
            return ERR("no parameter named '" + param.name + "'");
        }

        if (it->second.first != param)
        {
            return ERR("parameter with name '" + param.name + "' exists, but does not match provided parameter declaration");
        }

        return OK(it->second.second);
    }

    Result<Parameter> DataContainer::get_parameter_declaration(const std::string& name) const
    {
        if (auto it = m_parameters.find(name); it != m_parameters.end())
        {
            return OK(it->second.first);
        }

        return ERR("no parameter named '" + name + "'");
    }

    bool DataContainer::has_parameter(const std::string& name) const
    {
        return m_parameters.find(name) != m_parameters.end();
    }

    bool DataContainer::has_parameter(const Parameter& param) const
    {
        const auto it = m_parameters.find(param.name);
        return it != m_parameters.end() && it->second.first == param;
    }

    bool DataContainer::delete_parameter(const std::string& name)
    {
        return m_parameters.erase(name) > 0;
    }

    const std::unordered_map<std::string, std::pair<Parameter, std::string>>& DataContainer::get_parameters() const
    {
        return m_parameters;
    }

}    // namespace hal

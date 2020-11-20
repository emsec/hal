#include "hal_core/netlist/data_container.h"

#include "hal_core/utilities/log.h"

namespace hal
{
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

    std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> DataContainer::get_data_map() const
    {
        return m_data;
    }

    void DataContainer::set_data_map(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& map)
    {
        m_data = map;
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

}    // namespace hal

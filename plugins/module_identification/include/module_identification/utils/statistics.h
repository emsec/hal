#pragma once

#include "hal_core/defines.h"

#include "module_identification/candidates/base_candidate.h"
#include "module_identification/candidates/functional_candidate.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/json.hpp"

#include <map>
#include <mutex>
#include <set>
#include <string>

namespace hal
{
    class Gate;

    namespace module_identification
    {
        struct Statistics
        {
            std::map<const BaseCandidate*, std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>>> m_data;
            std::mutex m_mutex;

            void add_stat(const BaseCandidate* bc, const FunctionalCandidate& oc)
            {
                m_mutex.lock();

                // Check and insert for the base_candidate key
                if (m_data.find(bc) == m_data.end())
                {
                    m_data.insert({bc, std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>>()});
                }

                for (const auto& [processing_step, entries] : oc.m_timings)
                {
                    for (const auto& [type, entry] : entries)
                    {
                        for (const auto& [category, metrics] : entry)
                        {
                            for (const auto& [name, value] : metrics)
                            {
                                // std::cout << type << " - " << category << " - " << name << std::endl;
                                m_data[bc][processing_step][type][category][name] += value;
                            }
                        }
                    }
                }

                m_mutex.unlock();
            };

            void add_stat(const BaseCandidate* bc, const std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>>& stats)
            {
                m_mutex.lock();

                // Check and insert for the base_candidate key
                if (m_data.find(bc) == m_data.end())
                {
                    m_data.insert({bc, std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>>()});
                }

                for (const auto& [processing_step, entries] : stats)
                {
                    for (const auto& [type, entry] : entries)
                    {
                        for (const auto& [category, metrics] : entry)
                        {
                            for (const auto& [name, value] : metrics)
                            {
                                // std::cout << type << " - " << category << " - " << name << std::endl;
                                m_data[bc][processing_step][type][category][name] += value;
                            }
                        }
                    }
                }

                m_mutex.unlock();
            };

            void print()
            {
                std::cout << to_json() << std::endl;
            };

            std::string to_json(const u32 indent = 0)
            {
                std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>>> m_json_data;
                for (const auto& [bc, stats] : m_data)
                {
                    const auto bc_str = bc->m_gates.front()->get_name() + " - " + std::to_string(reinterpret_cast<std::uintptr_t>(bc));
                    for (const auto& [processing_step, entries] : stats)
                    {
                        for (const auto& [type, entry] : entries)
                        {
                            for (const auto& [category, metrics] : entry)
                            {
                                for (const auto& [name, value] : metrics)
                                {
                                    m_json_data[bc_str][processing_step][type][category][name] = value;
                                    m_json_data[bc_str][processing_step]["TOTAL"][category][name] += value;
                                    m_json_data["TOTAL"][processing_step][type][category][name] += value;
                                    m_json_data["TOTAL"][processing_step]["TOTAL"][category][name] += value;
                                }
                            }
                        }
                    }
                }

                nlohmann::json j = {m_json_data};

                return j.dump(indent);
            }
        };

    }    // namespace module_identification
}    // namespace hal
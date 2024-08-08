#include "module_identification/api/configuration.h"

namespace hal
{
    namespace module_identification
    {
        Configuration::Configuration(Netlist* nl) : m_netlist(nl){};

        Configuration::Configuration(){};

        Configuration& Configuration::with_known_registers(const std::vector<std::vector<hal::Gate*>>& registers)
        {
            m_known_registers = registers;
            return *this;
        }

        Configuration& Configuration::with_max_thread_count(const u32& max_thread_count)
        {
            m_max_thread_count = m_max_thread_count;
            return *this;
        }

        Configuration& Configuration::with_max_control_signals(const u32& max_control_signals)
        {
            m_max_control_signals = max_control_signals;
            return *this;
        }

        Configuration& Configuration::with_multithreading_priority(const MultithreadingPriority& priority)
        {
            m_multithreading_priority = priority;
            return *this;
        }

        Configuration& Configuration::with_types_to_check(const std::vector<hal::module_identification::CandidateType>& types_to_check)
        {
            m_types_to_check = types_to_check;
            return *this;
        }

        Configuration& Configuration::with_already_classified_candidates(const std::vector<std::vector<hal::Gate*>>& already_classified_candidates)
        {
            m_already_classified_candidates = already_classified_candidates;
            return *this;
        }

        Configuration& Configuration::with_blocked_base_candidates(const std::vector<std::set<hal::Gate*>>& blocked_base_candidates)
        {
            m_blocked_base_candidates = blocked_base_candidates;
            return *this;
        }
    }    // namespace module_identification

    template<>
    std::map<module_identification::CandidateType, std::string> EnumStrings<module_identification::CandidateType>::data = {
        {module_identification::CandidateType::adder, "ADDER"},
        {module_identification::CandidateType::absolute, "ABSOLUTE"},
        {module_identification::CandidateType::subtraction, "SUBTRACTION"},
        {module_identification::CandidateType::equal, "EQUAL"},
        {module_identification::CandidateType::less_equal, "LESS_EQUAL"},
        {module_identification::CandidateType::less_than, "LESS_THAN"},
        {module_identification::CandidateType::constant_multiplication, "CONSTANT_MULTIPLICATION"},
        {module_identification::CandidateType::counter, "COUNTER"},
        {module_identification::CandidateType::value_check, "VALUE_CHECK"},
        {module_identification::CandidateType::negation, "NEGATION"},
        {module_identification::CandidateType::none, "NONE"},
        {module_identification::CandidateType::mixed, "MIXED"},
        {module_identification::CandidateType::signed_less_than, "SIGNED_LESS_THAN"},
        {module_identification::CandidateType::signed_less_equal, "SIGNED_LESS_EQUAL"},
    };

}    // namespace hal
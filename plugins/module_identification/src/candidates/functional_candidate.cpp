#include "module_identification/candidates/functional_candidate.h"

#include "boolean_influence/include/boolean_influence/plugin_boolean_influence.h"
#include "hal_core/utilities/log.h"
#include "module_identification/candidates/base_candidate.h"
#include "module_identification/utils/utils.h"

namespace hal
{
    namespace module_identification
    {
        FunctionalCandidate::FunctionalCandidate(StructuralCandidate* sc, u32 max_control_signal, module_identification::CandidateType candidate_type)
        {
            m_structural_candidate = sc;
            m_gates                = sc->m_gates;
            m_candidate_type       = candidate_type;
            m_base_gates           = sc->base_candidate->m_gates;

            m_input_nets  = get_input_nets(m_gates);
            m_output_nets = get_output_nets(m_gates);

            m_max_control_signals = max_control_signal;
            m_max_operands        = 0;
            m_ctrl_to_operand_net = nullptr;

            m_control_signals = {};
            m_control_mapping = {};

            m_influence_count_to_input_nets = {};
            m_input_count_to_output_nets    = {};

            m_single_input_to_output = {};
            m_permuted_single_pairs  = {};

            m_sign_nets = {};

            m_operands = {};

            m_additional_data = {};

            m_timings = {};
        }

        void FunctionalCandidate::add_additional_data(std::string key, std::string value)
        {
            if (m_additional_data.find(key) != m_additional_data.end())
            {
                m_additional_data[key] = m_additional_data[key] + "\n" + value;
            }
            else
            {
                m_additional_data[key] = value;
            }
        }

        std::map<std::string, std::string> FunctionalCandidate::get_all_additional_data() const
        {
            return m_additional_data;
        }

        std::string FunctionalCandidate::get_candidate_info() const
        {
            std::string result = "";
            //result += "Verified: " + std::to_string(m_verified) + "\n";
            result += "Candidate Type: " + enum_to_string(m_candidate_type) + "\n";
            for (const auto& nets : m_operands)
            {
                result += "OPERAND [" + std::to_string(nets.size()) + "]" + ": \n";
                for (const auto& net : nets)
                {
                    result += "\t" + net->get_name() + " - " + std::to_string(net->get_id()) + "\n";
                }
            }
            result += "OUTPUT: [" + std::to_string(m_output_nets.size()) + "]\n";
            for (auto n : m_output_nets)
            {
                result += "\t" + n->get_name() + " / " + std::to_string(n->get_id()) + "\n";
            }
            result += "CONTROL SIGNALS: [" + std::to_string(m_control_signals.size()) + "]\n";
            result += "\t" + utils::join(", ", m_control_signals, [](const auto& n) { return n->get_name(); }) + "\n";
            result += "CONTROL MAPPING: \n";
            for (const auto& [n, v] : m_control_mapping)
            {
                result += "\t" + n->get_name() + " / " + std::to_string(n->get_id()) + " : " + std::to_string(v) + "\n";
            }
            // result += "ADDITIONAL DATA: \n";
            // for (const auto& [key, data] : m_additional_data)
            // {
            //     result += "\t" + key + ": " + data + "\n";
            // }

            return result;
        }

    }    // namespace module_identification
}    // namespace hal
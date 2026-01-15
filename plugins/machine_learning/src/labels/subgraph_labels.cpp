#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "machine_learning/labels/subgraph_label.h"
#include "machine_learning/types.h"

#include <stdlib.h>

namespace hal
{
    namespace machine_learning
    {
        namespace subgraph_label
        {
            Result<std::vector<std::vector<Gate*>>> ContainedComponents::calculate_subgraphs(Context& ctx) const
            {
                const auto& mbi = ctx.get_multi_bit_information();

                UNUSED(mbi);

                std::vector<std::vector<Gate*>> subgraphs;

                return OK(subgraphs);
            };

            Result<std::vector<std::vector<u32>>> ContainedComponents::calculate_labels(Context& ctx, const std::vector<Gate*>& subgraphs) const
            {
                std::vector<std::vector<u32>> labels;

                UNUSED(ctx);
                UNUSED(subgraphs);

                return OK(labels);
            }

            Result<u32> ContainedComponents::annotate_from_twin_netlist(Context& ctx, Netlist* nl, const Netlist* twin_nl) const
            {
                // find register components in twin nl
                const std::vector<std::string> SEQ_PATTERNS = {"Register"};
                std::vector<Module*> sequential_modules;

                for (const auto& m : twin_nl->get_modules())
                {
                    for (const auto& pat : SEQ_PATTERNS)
                    {
                        const auto& m_name = m->get_name();
                        if (m_name.find(pat) != std::string::npos)
                        {
                            {
                                sequential_modules.push_back(m);
                                break;
                            }
                        }
                    }
                }

                // find corresponding register in regular netlist
                const auto& mbi = ctx.get_multi_bit_information();

                for (const auto& m : sequential_modules)
                {
                    std::cout << "Searching for " << m->get_name() << std::endl;
                    std::vector<std::string> matched_words;
                    for (const auto& [word, _gates] : mbi.word_to_gates)
                    {
                        const auto& [wordname, _, __] = word;
                        std::cout << "\t" << wordname << " - " << _ << " - " << __ << std::endl;

                        if (wordname.find(m->get_name()) != std::string::npos)
                        {
                            matched_words.push_back(wordname);
                            std::cout << "Found match for " << wordname << std::endl;
                        }
                    }

                    if (matched_words.empty())
                    {
                        log_warning("machine_learning", "Failed to find matching word for sequential module name {} in twin netlist", m->get_name());
                    }

                    if (matched_words.size() > 1)
                    {
                        log_warning("machine_learning", "Found multiple ({}) matching words for sequential module name {} in twin netlist", matched_words.size(), m->get_name());
                    }
                }

                // build combinational subgraph in front of twin reg

                // check for components in combinational subgraph

                // annotate list of found components to data container of corresponding netlist

                return OK(0);
            }

            std::string ContainedComponents::to_string() const
            {
                return "ContainedComponents";
            }
        }    // namespace subgraph_label
    }    // namespace machine_learning
}    // namespace hal
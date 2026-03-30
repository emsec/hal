#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "machine_learning/labels/subgraph_label.h"
#include "machine_learning/types.h"

#include <fstream>
#include <nlohmann/json.hpp>
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

            Result<std::vector<std::vector<u32>>> ContainedComponents::calculate_labels(Context& ctx, const std::vector<std::vector<Gate*>>& subgraphs) const
            {
                std::vector<std::vector<u32>> labels;

                UNUSED(ctx);
                UNUSED(subgraphs);

                return OK(labels);
            }

            Result<u32> ContainedComponents::annotate_from_twin_netlist(Context& ctx, Netlist* nl, const Netlist* twin_nl) const
            {
                UNUSED(nl);

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
                std::string key_words_to_str = utils::join("__",    // separator between groups (double underscore for clarity)
                                                           m_key_words.begin(),
                                                           m_key_words.end(),
                                                           [](const std::vector<std::string>& group) { return utils::join("_", group.begin(), group.end(), [](const std::string& s) { return s; }); });

                return "ContainedComponents" + key_words_to_str + "_" + (m_binary_flags ? "binary" : "count");
            }

            Result<std::vector<std::vector<Gate*>>> ContainedComponentsNetlist::calculate_subgraphs(Context& ctx) const
            {
                const std::vector<std::vector<Gate*>> subgraphs = {ctx.nl->get_gates()};

                return OK(subgraphs);
            };

            Result<std::vector<std::vector<u32>>> ContainedComponentsNetlist::calculate_labels(Context& ctx, const std::vector<std::vector<Gate*>>& subgraphs) const
            {
                UNUSED(subgraphs);    // For netlist-level labeling, the subgraph is always the full netlist

                Module* top_module = ctx.nl->get_top_module();
                if (top_module == nullptr)
                {
                    return ERR("netlist has no top module — cannot read component annotations");
                }

                const std::string CATEGORY = "ContainedComponentsNetlist";

                // Match each keyword group against the annotations stored in the top module
                std::vector<std::pair<u32, u32>> matches;

                for (u32 kw_idx = 0; kw_idx < m_key_words.size(); ++kw_idx)
                {
                    u32 group_count = 0;

                    for (const auto& keyword : m_key_words[kw_idx])
                    {
                        if (top_module->has_data(CATEGORY, keyword))
                        {
                            const auto [type, value] = top_module->get_data(CATEGORY, keyword);

                            auto stoul_res = utils::wrapped_stoul(value);
                            if (stoul_res.is_error())
                            {
                                return ERR("failed to parse stored value '" + value + "' for keyword '" + keyword + "': " + stoul_res.get_error().get());
                            }
                            group_count += stoul_res.get();
                        }
                    }

                    if (group_count > 0)
                    {
                        matches.push_back({kw_idx, group_count});
                    }
                }

                // Build the label vector for this (single) netlist-level subgraph
                std::vector<u32> label;
                if (matches.empty())
                {
                    label = make_no_match();
                }
                else
                {
                    auto label_res = make_match(matches);
                    if (label_res.is_error())
                    {
                        return ERR(label_res.get_error().get());
                    }
                    label = label_res.get();
                }

                // calculate_subgraphs returns one subgraph (all gates), so one label entry
                return OK(std::vector<std::vector<u32>>{label});
            }

            Result<u32> ContainedComponentsNetlist::annotate_from_netlist_metadata(Context& ctx, Netlist* nl, const std::string& metadata_path) const
            {
                UNUSED(ctx);

                std::ifstream file(metadata_path);
                if (!file.is_open())
                {
                    return ERR("cannot open metadata file at path: " + metadata_path);
                }

                nlohmann::json j;
                try
                {
                    file >> j;
                }
                catch (const std::exception& e)
                {
                    return ERR("failed to parse JSON metadata at '" + metadata_path + "': " + std::string(e.what()));
                }

                const std::string ROOT_KEY = "design_information";
                const std::string CATEGORY = "ContainedComponentsNetlist";

                if (!j.contains(ROOT_KEY))
                {
                    return ERR("metadata JSON does not contain the key '" + ROOT_KEY + "'");
                }

                const auto& design_information = j[ROOT_KEY];
                if (!design_information.is_object())
                {
                    return ERR("metadata JSON key '" + ROOT_KEY + "' does not contain an object");
                }
                if (!design_information.contains(CATEGORY))
                {
                    return ERR("metadata JSON does not contain the key path '" + ROOT_KEY + "." + CATEGORY + "'");
                }

                const auto& component_metadata = design_information[CATEGORY];
                if (!component_metadata.is_object())
                {
                    return ERR("metadata JSON key path '" + ROOT_KEY + "." + CATEGORY + "' does not contain an object");
                }

                Module* top_module = nl->get_top_module();
                if (top_module == nullptr)
                {
                    return ERR("netlist has no top module");
                }

                u32 annotated_count = 0;
                for (auto it = component_metadata.begin(); it != component_metadata.end(); ++it)
                {
                    const std::string& component_name = it.key();

                    if (!it.value().is_number_unsigned())
                    {
                        return ERR("value for component '" + component_name + "' is not an unsigned integer in metadata file: " + metadata_path);
                    }

                    const u32 component_count = it.value().get<u32>();

                    if (!top_module->set_data(CATEGORY, component_name, "integer", std::to_string(component_count)))
                    {
                        return ERR("failed to annotate component '" + component_name + "' to top module data container");
                    }
                    annotated_count++;
                }

                return OK(annotated_count);
            }

            std::string ContainedComponentsNetlist::to_string() const
            {
                std::string key_words_to_str = utils::join("__",    // separator between groups (double underscore for clarity)
                                                           m_key_words.begin(),
                                                           m_key_words.end(),
                                                           [](const std::vector<std::string>& group) { return utils::join("_", group.begin(), group.end(), [](const std::string& s) { return s; }); });

                return "ContainedComponentsNetlist" + key_words_to_str + "_" + (m_binary_flags ? "binary" : "count");
            }
        }    // namespace subgraph_label
    }    // namespace machine_learning
}    // namespace hal

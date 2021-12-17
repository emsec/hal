#include "hal_core/netlist/boolean_function/parser.h"

namespace hal {
	std::variant<BooleanFunction, std::string> BooleanFunction::quine_mccluskey(const BooleanFunction& function) 
	{
		/// TODO: implement me based on the old implementation for single-bit Boolean functions, see below.
		return function;
	}
/*
    std::vector<std::vector<BooleanFunction::Value>> BooleanFunction::qmc(std::vector<std::vector<Value>> terms)
    {
        if (terms.empty())
        {
            return {};
        }

        u32 num_variables = terms[0].size();

        // repeatedly merge all groups that only differ in a single value
        while (true)
        {
            bool any_changes = false;
            std::vector<std::vector<Value>> new_merged_terms;
            std::vector<bool> removed_by_merge(terms.size(), false);
            // pairwise compare all terms...
            for (u32 i = 0; i < terms.size(); ++i)
            {
                for (u32 j = i + 1; j < terms.size(); ++j)
                {
                    // ...merge their values and count differences...
                    std::vector<Value> merged(num_variables, Value::X);
                    u32 cnt = 0;
                    for (u32 k = 0; k < num_variables; ++k)
                    {
                        if (terms[i][k] == terms[j][k])
                        {
                            merged[k] = terms[i][k];
                        }
                        else
                        {
                            ++cnt;
                        }
                    }
                    // ...and if they differ only in a single value, replace them with the merged term
                    if (cnt == 1)
                    {
                        removed_by_merge[i] = removed_by_merge[j] = true;
                        new_merged_terms.push_back(merged);
                        any_changes = true;
                    }
                }
            }
            if (!any_changes)
            {
                break;
            }
            for (u32 i = 0; i < terms.size(); ++i)
            {
                if (!removed_by_merge[i])
                {
                    new_merged_terms.push_back(terms[i]);
                }
            }
            std::sort(new_merged_terms.begin(), new_merged_terms.end());
            new_merged_terms.erase(std::unique(new_merged_terms.begin(), new_merged_terms.end()), new_merged_terms.end());
            terms = new_merged_terms;
        }

        std::vector<std::vector<Value>> output;

        // build ON-set minterms to later identify essential implicants
        std::map<u32, std::vector<u32>> table;
        for (u32 i = 0; i < terms.size(); ++i)
        {
            // find all inputs that are covered by term i
            // Example: term=01-1 --> all inputs covered are 0101 and 0111
            std::vector<u32> covered_inputs;
            for (u32 j = 0; j < num_variables; ++j)
            {
                if (terms[i][j] != Value::X)
                {
                    if (covered_inputs.empty())
                    {
                        covered_inputs.push_back(terms[i][j]);
                    }
                    else
                    {
                        for (auto& v : covered_inputs)
                        {
                            v = (v << 1) | terms[i][j];
                        }
                    }
                }
                else
                {
                    if (covered_inputs.empty())
                    {
                        covered_inputs.push_back(0);
                        covered_inputs.push_back(1);
                    }
                    else
                    {
                        std::vector<u32> tmp;
                        for (auto v : covered_inputs)
                        {
                            tmp.push_back((v << 1) | 0);
                            tmp.push_back((v << 1) | 1);
                        }
                        covered_inputs = tmp;
                    }
                }
            }

            std::sort(covered_inputs.begin(), covered_inputs.end());
            covered_inputs.erase(std::unique(covered_inputs.begin(), covered_inputs.end()), covered_inputs.end());

            // now memorize that all these inputs are covered by term i
            for (auto v : covered_inputs)
            {
                table[v].push_back(i);
            }
        }

        // helper function to add a term to the output and remove it from the table
        auto add_to_output = [&](u32 term_index) {
            output.push_back(terms[term_index]);
            for (auto it2 = table.cbegin(); it2 != table.cend();)
            {
                if (std::find(it2->second.begin(), it2->second.end(), term_index) != it2->second.end())
                {
                    it2 = table.erase(it2);
                }
                else
                {
                    ++it2;
                }
            }
        };

        // finally, identify essential implicants and add them to the output
        while (!table.empty())
        {
            bool no_change = true;

            for (auto& it : table)
            {
                if (it.second.size() == 1)
                {
                    no_change = false;
                    add_to_output(it.second[0]);
                    break;    // 'add_to_output' invalidates table iterator, so break and restart in next iteration
                }
            }

            if (no_change)
            {
                // none of the remaining terms is essential, just pick the one that covers most input values
                std::unordered_map<u32, u32> counter;
                for (auto it : table)
                {
                    for (auto term_index : it.second)
                    {
                        counter[term_index]++;
                    }
                }

                u32 index = std::max_element(counter.begin(), counter.end(), [](const auto& p1, const auto& p2) { return p1.second < p2.second; })->first;
                add_to_output(index);
            }
        }

        // sort output terms for deterministic output order (no impact on functionality)
        std::sort(output.begin(), output.end());

        return output;
    }
*/

}  // namespace hal
#include "dataflow_analysis/output_generation/dot_graph.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/utilities/log.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace hal
{
    namespace dataflow
    {
        namespace dot_graph
        {
            namespace
            {
                std::string generate_dot_graph(const std::shared_ptr<Grouping>& state)
                {
                    auto netlist_abstr = state->netlist_abstr;

                    std::stringstream dot_graph;
                    dot_graph << "digraph {\n\tnode [shape=box fillcolor=white style=filled];\n";

                    // print node
                    for (const auto& [group_id, gates] : state->gates_of_group)
                    {
                        auto size = gates.size() * 0.1;
                        if (size < 1500)
                        {
                            size = size * 0.02;
                        }
                        //dot_graph << group_id << " [width=" << 0.1 * gates.size() << " label=\"" << gates.size() << " bit (id " << group_id << ")\"];\n";
                        dot_graph << group_id << " [width=" << size << " label=\"" << gates.size() << " bit \"];\n";
                    }
                    // print edges
                    for (const auto& [group_id, gates] : state->gates_of_group)
                    {
                        for (auto suc : state->get_successor_groups_of_group(group_id))
                        {
                            dot_graph << group_id << " -> " << suc << ";\n";
                        }
                    }
                    dot_graph << "}";
                    return dot_graph.str();

                    return std::string();
                }
            }    // namespace

            bool create_graph(const std::shared_ptr<Grouping>& state, const std::string path, const std::vector<std::string>& file_types)
            {
                if (file_types.empty())
                {
                    return true;
                }

                std::string path_dot_file = path + "dot_graph.dot";

                // create dot file
                {
                    std::string dot_graph = generate_dot_graph(state);
                    log_info("dataflow", "creating dot file...");
                    std::ofstream dot_file(path_dot_file, std::ofstream::out);
                    dot_file << dot_graph;
                    dot_file.close();
                }

                // generate output files
                for (const auto& file_type : file_types)
                {
                    log_info("dataflow", "generating " + file_type + "...");
                    measure_block_time(file_type + " generation");

                    std::string path_graph_file = path + "dot_graph." + file_type;

                    std::string command = "dot -T" + file_type + " " + path_dot_file + " > " + path_graph_file;
                    int status          = std::system(command.c_str());
                    if (status != 0)
                    {
                        log_error("dataflow", "graph couldn't be generated, did you install graphviz?; return code: {}", status);
                        return false;
                    }
                    log_info("dataflow", "finished: {}", path_graph_file);
                }

                return true;
            }
        }    // namespace dot_graph
    }        // namespace dataflow
}    // namespace hal
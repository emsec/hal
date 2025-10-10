#include "clock_tree_extractor/clock_tree_extractor.h"

#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/result.h"

#include <fstream>
#include <queue>

namespace hal
{
    namespace cte
    {
        namespace
        {
            bool continue_through_exit_ep( const Endpoint *exit_ep, const u32 current_depth )
            {
                if( exit_ep == nullptr )
                {
                    return false;
                }

                const Gate *gate = exit_ep->get_gate();
                const GateType *gate_type = gate->get_type();
                const GateTypeProperty ff = GateTypeProperty::ff;
                const GatePin *gate_pin = exit_ep->get_pin();

                if( gate_type->has_property( ff ) && current_depth == 0 )
                {
                    return gate_pin->get_direction() == PinDirection::input && gate_pin->get_type() == PinType::clock;
                }

                return !( gate_type->has_property( ff ) && current_depth > 0 );
            }

            void export_clock_tree( const std::unordered_map<std::string, std::unordered_set<std::string>> &vertices,
                                    const std::vector<std::pair<std::string, std::string>> edges,
                                    const std::string &pathname )
            {
                if( pathname == "" )
                {
                    return;
                }

                std::ofstream dot_fd( pathname );

                if( !dot_fd )
                {
                    log_error( "clock_tree_extractor", "could not export clock tree to '{}'", pathname );
                    return;
                }

                dot_fd << "digraph {\n";

                if( vertices.find( "global_inputs" ) != vertices.end() )
                {
                    for( const std::string &net_name : vertices.at( "global_inputs" ) )
                    {
                        dot_fd << "  " << net_name << " [color=green];\n";
                    }
                }

                if( vertices.find( "ffs" ) != vertices.end() )
                {
                    for( const std::string &gate_id : vertices.at( "ffs" ) )
                    {
                        dot_fd << "  " << gate_id << " [color=dodgerblue];\n";
                    }
                }

                if( vertices.find( "buffers" ) != vertices.end() )
                {
                    for( const std::string &gate_id : vertices.at( "buffers" ) )
                    {
                        dot_fd << "  " << gate_id << " [color=gold];\n";
                    }
                }

                for( const auto &[src, dst] : edges )
                {
                    const std::string edge_color =
                        ( vertices.at( "buffers" ).find( src ) != vertices.at( "buffers" ).end() ) ? "gold" : "green";
                    dot_fd << "  " << src << " -> " << dst << " [color=" << edge_color << "];";
                }

                dot_fd << "}\n";
                dot_fd.close();

                log_info( "clock_tree_extractor", "successfully exported clock tree to '{}'", pathname );
            }
        }  // namespace

        ClockTreeExtractor::ClockTreeExtractor( const Netlist *netlist )
        {
            m_netlist = netlist;
        }

        Result<u32> ClockTreeExtractor::analyze( const std::string &pathname = "" )
        {
            if( m_netlist == nullptr )
            {
                return ERR( "no netlist provided" );
            }

            std::queue<const Gate *> queue;
            std::unordered_set<const Gate *> visited;
            std::vector<std::pair<std::string, std::string>> edges;
            std::unordered_map<std::string, std::unordered_set<std::string>> vertices;
            NetlistTraversalDecorator ntd = NetlistTraversalDecorator( *m_netlist );

            const std::vector<Gate *> start_gates = m_netlist->get_gates(
                []( const Gate *g ) { return g->get_type()->has_property( GateTypeProperty::ff ); } );

            for( const Gate *gate : start_gates )
            {
                queue.push( gate );
                const std::string gate_id = std::to_string( gate->get_id() );
                vertices["ffs"].insert( gate_id );
            }

            while( !queue.empty() )
            {
                const Gate *current = queue.front();
                const std::string current_id = std::to_string( current->get_id() );
                queue.pop();

                if( auto it = visited.find( current ); it != visited.end() )
                {
                    continue;
                }

                visited.insert( current );

                auto result = ntd.get_next_matching_gates(
                    current,
                    false,
                    []( const Gate *g ) { return g->get_type()->has_property( GateTypeProperty::c_buffer ); },
                    false,
                    continue_through_exit_ep,
                    nullptr );
                if( result.is_error() )
                {
                    return ERR( result.get_error() );
                }
                const std::set<Gate *> next_matching_gates = result.get();

                if( next_matching_gates.empty() )
                {
                    continue;
                }

                for( const Gate *predecessor : next_matching_gates )
                {
                    const std::string predecessor_id = std::to_string( predecessor->get_id() );
                    std::pair<std::string, std::string> edge = std::make_pair( predecessor_id, current_id );
                    if( std::find( edges.begin(), edges.end(), edge ) != edges.end() )
                    {
                        continue;
                    }
                    edges.push_back( edge );
                    queue.push( predecessor );
                    vertices["buffers"].insert( predecessor_id );
                }
            }

            for( const Net *net : m_netlist->get_global_input_nets() )
            {
                const std::string net_name = net->get_name();

                auto result = ntd.get_next_matching_gates(
                    net,
                    true,
                    []( const Gate *g ) { return g->get_type()->has_property( GateTypeProperty::c_buffer ); },
                    false,
                    continue_through_exit_ep,
                    nullptr );
                if( result.is_error() )
                {
                    return ERR( result.get_error() );
                }
                const std::set<Gate *> next_matching_gates = result.get();

                std::size_t childs = 0;
                for( const Gate *gate : next_matching_gates )
                {
                    if( auto it = visited.find( gate ); it == visited.end() )
                    {
                        continue;
                    }

                    const u32 gate_id = gate->get_id();
                    const std::string gate_id_str = std::to_string( gate_id );

                    const std::pair<std::string, std::string> edge = std::make_pair( net_name, gate_id_str );

                    if( std::find( edges.begin(), edges.end(), edge ) != edges.end() )
                    {
                        continue;
                    }

                    edges.push_back( edge );
                    childs++;
                }

                if( childs > 0 )
                {
                    vertices["global_inputs"].insert( net_name );
                }
            }

            export_clock_tree( vertices, edges, pathname );

            m_edges = edges;

            return OK( edges.size() );
        }

        const std::vector<std::pair<std::string, std::string>> ClockTreeExtractor::get_edges() const
        {
            return m_edges;
        }
    }  // namespace cte
}  // namespace hal

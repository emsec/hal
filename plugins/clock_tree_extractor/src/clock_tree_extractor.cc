#include "clock_tree_extractor/clock_tree_extractor.h"

#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/result.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    class GatePin;
}

namespace hal
{
    namespace cte
    {
        namespace
        {
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
                    dot_fd << "  " << src << " -> " << dst << " [color=" << edge_color << "];\n";
                }

                dot_fd << "}\n";
                dot_fd.close();

                log_info( "clock_tree_extractor", "successfully exported clock tree to '{}'", pathname );
            }

            struct GatePairHash
            {
                std::size_t operator()( const std::pair<const Gate *, const Gate *> &pair ) const
                {
                    return std::hash<const Gate *>()( pair.first ) ^ ( std::hash<const Gate *>()( pair.second ) << 1 );
                }
            };

            bool is_control_pin( const PinType &pin_type )
            {
                return pin_type == PinType::clock || pin_type == PinType::enable || pin_type == PinType::select
                    || pin_type == PinType::set || pin_type == PinType::reset;
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

            std::queue<std::pair<const Gate *, const Gate *>> queue;
            NetlistTraversalDecorator ntd = NetlistTraversalDecorator( *m_netlist );
            std::unordered_set<std::pair<const Gate *, const Gate *>, GatePairHash> visited;
            std::vector<std::pair<std::string, std::string>> edges;  // TODO: use unordered_set
            std::unordered_map<std::string, std::unordered_set<std::string>> vertices;

            const std::vector<Gate *> ffs = m_netlist->get_gates(
                []( const Gate *g ) { return g->get_type()->has_property( GateTypeProperty::ff ); } );

            for( const Gate *ff : ffs )
            {
                const u32 ff_id = ff->get_id();
                const std::string ff_name = ff->get_name();
                const std::string ff_id_str = std::to_string( ff_id );

                vertices["ffs"].insert( ff_id_str );

                const std::vector<hal::GatePin *> clock_pins = ff->get_type()->get_pins( []( const auto &p ) {
                    return ( p->get_direction() == PinDirection::input ) && ( p->get_type() == PinType::clock );
                } );
                if( clock_pins.size() != 1 )
                {
                    log_error( "clock_tree_extractor",
                               "invalid number of input clock pins at gate '" + ff_name + "' with ID " + ff_id_str );
                    continue;
                }

                const GatePin *clock_pin = clock_pins.front();
                const Net *clk = ff->get_fan_in_net( clock_pin );
                if( clk == nullptr )
                {
                    log_error( "clock_tree_extractor",
                               "no net connected to clock pin at gate '" + ff_name + "' with ID " + ff_id_str );
                    continue;
                }

                const u32 clk_id = clk->get_id();
                const std::string clk_id_str = std::to_string( clk_id );
                const std::vector<hal::Endpoint *> clk_sources = clk->get_sources();

                if( clk_sources.size() > 1 )
                {
                    log_error( "clock_tree_extractor",
                               "invalid number of sources for clock net with ID " + clk_id_str );
                    continue;
                }
                else if( clk->is_global_input_net() )
                {
                    const std::string clk_name = clk->get_name();
                    vertices["global_inputs"].insert( clk_name );
                    continue;
                }
                else if( clk_sources.size() == 0 )
                {
                    // ignore unrouted nets for now
                    continue;
                }

                const Endpoint *source_ep = clk_sources.front();
                const Gate *source_gate = source_ep->get_gate();

                queue.push( { ff, source_gate } );
            }

            while( !queue.empty() )
            {
                const std::pair<const Gate *, const Gate *> pair = queue.front();
                queue.pop();

                Gate *source = (Gate *) pair.second;
                Gate *reference = (Gate *) pair.first;

                if( source->get_type()->has_property( GateTypeProperty::c_buffer ) )
                {
                    const u32 source_id = source->get_id();
                    const u32 reference_id = reference->get_id();
                    const std::string source_id_str = std::to_string( source_id );
                    const std::string reference_id_str = std::to_string( reference_id );
                    const std::pair<std::string, std::string> edge{ source_id_str, reference_id_str };

                    if( auto it = std::find( edges.begin(), edges.end(), edge ); it == edges.end() )
                    {
                        edges.push_back( edge );
                    }

                    reference = (Gate *) source;

                    const u32 buffer_id = source->get_id();
                    const std::string buffer_id_str = std::to_string( buffer_id );

                    vertices["buffers"].insert( buffer_id_str );
                }
                else if( source->get_type()->has_property( GateTypeProperty::ff ) )
                {
                    // Should the FFS also be traversed? If so, which inputs should be considered? I would assume only
                    // the data inputs.
                    continue;
                }

                visited.insert( pair );

                for( const Endpoint *ep : source->get_fan_in_endpoints() )
                {
                    const PinType pin_type = ep->get_pin()->get_type();
                    if( is_control_pin( pin_type ) )
                    {
                        // Don't traverse control signals of clock gates
                        continue;
                    }

                    const Net *net = ep->get_net();
                    if( net->is_global_input_net() )
                    {
                        const u32 reference_id = reference->get_id();
                        const std::string net_name = net->get_name();
                        const std::string reference_id_str = std::to_string( reference_id );
                        const std::pair<std::string, std::string> edge{ net_name, reference_id_str };

                        if( auto it = std::find( edges.begin(), edges.end(), edge ); it == edges.end() )
                        {
                            edges.push_back( edge );
                        }

                        vertices["global_inputs"].insert( net_name );
                        continue;
                    }

                    for( const Endpoint *ep : net->get_sources() )
                    {
                        const Gate *gate = ep->get_gate();
                        if( auto it = visited.find( { reference, gate } ); it == visited.end() )
                        {
                            queue.push( { reference, gate } );
                        }
                    }
                }
            }

            m_edges = edges;
            export_clock_tree( vertices, edges, pathname );

            return OK( edges.size() );
        }

        const std::vector<std::pair<std::string, std::string>> ClockTreeExtractor::get_edges() const
        {
            return m_edges;
        }
    }  // namespace cte
}  // namespace hal

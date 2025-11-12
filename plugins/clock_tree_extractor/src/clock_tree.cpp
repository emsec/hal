#include "clock_tree_extractor/clock_tree.h"

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
#include <igraph/igraph.h>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    namespace cte
    {
        namespace
        {
            struct VoidPtrHash
            {
                std::size_t operator()( const std::pair<const void *, const void *> &pair ) const
                {
                    return std::hash<const void *>()( pair.first ) ^ ( std::hash<const void *>()( pair.second ) << 1 );
                }
            };

            struct PairPtrEq
            {
                bool operator()( const std::pair<void *, void *> &p1,
                                 const std::pair<void *, void *> &p2 ) const noexcept
                {
                    return p1.first == p2.first && p1.second == p2.second;
                }
            };

            inline bool is_ff( const Gate *gate )
            {
                return gate->get_type()->has_property( GateTypeProperty::ff );
            }

            inline bool is_latch( const Gate *gate )
            {
                return gate->get_type()->has_property( GateTypeProperty::latch );
            }

            inline bool is_buffer( const Gate *gate )
            {
                return gate->get_type()->has_property( GateTypeProperty::c_buffer );
            }

            inline bool is_inverter( const Gate *gate )
            {
                return gate->get_type()->has_property( GateTypeProperty::c_inverter );
            }

            inline bool is_control_pin( const PinType &pin_type )
            {
                return pin_type == PinType::clock || pin_type == PinType::enable || pin_type == PinType::select
                    || pin_type == PinType::set || pin_type == PinType::reset;
            }

            inline bool is_connected_to_control_pin( const Endpoint *endpoint )
            {
                return is_control_pin( endpoint->get_pin()->get_type() );
            }

            const std::unordered_set<const Gate *> get_toggle_ffs( const Netlist *netlist )
            {
                const std::vector<Gate *> ffs = netlist->get_gates( is_ff );

                std::unordered_set<const Gate *> result;
                for( const Gate *ff : ffs )
                {
                    const std::vector<Endpoint *> successor_endpoints = ff->get_successors();
                    const std::size_t successor_endpoints_size = successor_endpoints.size();

                    if( successor_endpoints_size == 0 )
                    {
                        continue;
                    }

                    std::vector<const Gate *> successors;
                    successors.reserve( successor_endpoints_size );

                    std::transform( successor_endpoints.begin(),
                                    successor_endpoints.end(),
                                    std::back_inserter( successors ),
                                    []( const Endpoint *ep ) { return ep->get_gate(); } );

                    if( std::find( successors.begin(), successors.end(), ff ) != successors.end() )
                    {
                        result.insert( ff );
                    }
                }

                return result;
            }
        }  // namespace

        ClockTree::ClockTree( const Netlist *netlist )
            : m_netlist( netlist )
            , m_igraph_ptr( &m_igraph )
        {
        }

        ClockTree::~ClockTree()
        {
            igraph_destroy( &m_igraph );
        }

        Result<std::unique_ptr<ClockTree>> ClockTree::from_netlist( const Netlist *netlist )
        {
            if( netlist == nullptr )
            {
                return ERR( "no netlist provided" );
            }

            std::unordered_set<void *> vertices;
            std::unordered_set<std::pair<void *, void *>, VoidPtrHash> edges;
            std::unordered_map<const void *, std::string> ptrs_to_type;

            std::queue<std::pair<const Gate *, const Gate *>> queue;
            NetlistTraversalDecorator ntd = NetlistTraversalDecorator( *netlist );
            std::unordered_set<std::pair<const Gate *, const Gate *>, VoidPtrHash> visited;

            for( const Gate *ff : netlist->get_gates( is_ff ) )
            {
                const std::vector<hal::GatePin *> clock_pins = ff->get_type()->get_pins( []( const auto &p ) {
                    return ( p->get_direction() == PinDirection::input ) && ( p->get_type() == PinType::clock );
                } );

                if( clock_pins.size() != 1 )
                {
                    log_error( "clock_tree_extractor",
                               "invalid number of input clock pins at gate '" + ff->get_name() + "' with ID "
                                   + std::to_string( ff->get_id() ) );
                    continue;
                }

                const Net *clk = ff->get_fan_in_net( clock_pins.front() );
                if( clk == nullptr )
                {
                    log_error( "clock_tree_extractor",
                               "no net connected to clock pin at gate '" + ff->get_name() + "' with ID "
                                   + std::to_string( ff->get_id() ) );
                    continue;
                }

                if( clk->get_num_of_sources() > 1 )
                {
                    log_error( "clock_tree_extractor",
                               "invalid number of sources for clock net with ID " + std::to_string( clk->get_id() ) );
                    continue;
                }
                else if( clk->is_global_input_net() )
                {
                    vertices.insert( (void *) clk );
                    ptrs_to_type[(void *) clk] = "net";
                    continue;
                }
                else if( clk->get_num_of_sources() == 0 )
                {
                    continue;
                }

                queue.push( { ff, clk->get_sources().front()->get_gate() } );

                vertices.insert( (void *) ff );
                ptrs_to_type[(void *) ff] = "gate";
            }

            const std::unordered_set<const Gate *> toggle_ffs = get_toggle_ffs( netlist );

            while( !queue.empty() )
            {
                const std::pair<const Gate *, const Gate *> pair = queue.front();
                queue.pop();

                Gate *source = (Gate *) pair.second;
                Gate *reference = (Gate *) pair.first;

                if( is_latch( source ) )
                {
                    // Ignore latches
                    continue;
                }
                else if( is_buffer( source ) || is_inverter( source ) || is_ff( source ) )
                {
                    if( is_ff( source ) && toggle_ffs.find( source ) == toggle_ffs.end() )
                    {
                        // Include only toggle flip-flops for now
                        continue;
                    }

                    vertices.insert( (void *) source );
                    vertices.insert( (void *) reference );

                    ptrs_to_type[(void *) source] = "gate";
                    ptrs_to_type[(void *) reference] = "gate";

                    edges.insert( { (void *) source, (void *) reference } );

                    if( is_ff( source ) )
                    {
                        continue;
                    }

                    reference = (Gate *) source;
                }

                visited.insert( pair );

                for( const Endpoint *ep : source->get_fan_in_endpoints() )
                {
                    if( is_connected_to_control_pin( ep ) )
                    {
                        // Don't traverse control signals of clock gates
                        continue;
                    }

                    const Net *net = ep->get_net();
                    if( net->get_name() == "'0'" || net->get_name() == "'1'" )
                    {
                        // Don't traverse power/ground signals
                        continue;
                    }

                    if( net->is_global_input_net() )
                    {
                        vertices.insert( (void *) net );
                        vertices.insert( (void *) reference );

                        ptrs_to_type[(void *) net] = "net";
                        ptrs_to_type[(void *) reference] = "gate";

                        edges.insert( { (void *) net, (void *) reference } );
                        continue;
                    }

                    if( net->get_num_of_sources() == 0 )
                    {
                        log_warning( "clock_tree_extractor",
                                     "unrouted clock net with ID {} ignored",
                                     std::to_string( net->get_id() ) );
                        continue;
                    }
                    else if( net->get_num_of_sources() > 1 )
                    {
                        log_warning( "clock_tree_extractor",
                                     "multi-driven clock net with ID {} ignored",
                                     std::to_string( net->get_id() ) );
                        continue;
                    }

                    const Gate *new_source = net->get_sources().front()->get_gate();
                    if( visited.find( { reference, new_source } ) == visited.end() )
                    {
                        queue.push( { reference, new_source } );
                    }
                }
            }

            std::unique_ptr<ClockTree> clock_tree = std::unique_ptr<ClockTree>( new ClockTree( netlist ) );

            igraph_integer_t idx = 0;
            for( const void *vertex : vertices )
            {
                const igraph_integer_t vertex_id = idx++;

                clock_tree->m_vertices_to_ptrs[vertex_id] = vertex;
                clock_tree->m_ptrs_to_vertices[vertex] = vertex_id;
            }

            clock_tree->m_ptrs_to_types = ptrs_to_type;

            igraph_error_t ierror;
            igraph_vector_int_t iedges;
            if( ( ierror = igraph_vector_int_init( &iedges, 2 * edges.size() ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            idx = 0;
            for( const auto &[src, dst] : edges )
            {
                VECTOR( iedges )[idx++] = clock_tree->m_ptrs_to_vertices.at( src );
                VECTOR( iedges )[idx++] = clock_tree->m_ptrs_to_vertices.at( dst );
            }

            if( ( ierror = igraph_create( clock_tree->m_igraph_ptr, &iedges, vertices.size(), IGRAPH_DIRECTED ) )
                != IGRAPH_SUCCESS )
            {
                igraph_vector_int_destroy( &iedges );
                return ERR( igraph_strerror( ierror ) );
            }

            igraph_vector_int_destroy( &iedges );

            igraph_vector_int_t indegrees;
            if( ( ierror = igraph_vector_int_init( &indegrees, 0 ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            if( ( ierror = igraph_degree(
                      clock_tree->m_igraph_ptr, &indegrees, igraph_vss_all(), IGRAPH_IN, IGRAPH_NO_LOOPS ) )
                != IGRAPH_SUCCESS )
            {
                igraph_vector_int_destroy( &indegrees );
                return ERR( igraph_strerror( ierror ) );
            }

            for( idx = 0; idx < igraph_vector_int_size( &indegrees ); idx++ )
            {
                if( VECTOR( indegrees )[idx] != 0 )
                {
                    continue;
                }
                clock_tree->m_roots.insert( idx );
            }

            igraph_vector_int_destroy( &indegrees );

            return OK( std::move( clock_tree ) );
        }

        Result<std::monostate> ClockTree::export_dot( const std::string &pathname ) const
        {
            std::ofstream dot_fd( pathname );

            if( !dot_fd )
            {
                return ERR( "couldn't export clock tree to '" + pathname + "'" );
            }

            dot_fd << "digraph { comment=\"created by HAL plugin clock_tree_extractor\"\n";

            for( const auto &[ptr, vertex] : m_ptrs_to_vertices )
            {
                if( m_ptrs_to_types.at( ptr ) == "net" )
                {
                    dot_fd << "  " << ( (Net *) ptr )->get_name() << " [shape=circle];\n";
                    continue;
                }

                const i32 x = ( (Gate *) ptr )->get_location_x();
                const i32 y = ( (Gate *) ptr )->get_location_y();

                const std::string coords =
                    ( x < 0 || y < 0 ) ? "" : "x=" + std::to_string( x ) + " y=" + std::to_string( y );

                if( is_buffer( (Gate *) ptr ) )
                {
                    dot_fd << "  " << std::to_string( ( (Gate *) ptr )->get_id() ) << " [" << coords
                           << " shape=rectangle];\n";
                }
                else if( is_inverter( (Gate *) ptr ) )
                {
                    dot_fd << "  " << std::to_string( ( (Gate *) ptr )->get_id() ) << " [" << coords
                           << " shape=triangle, orientation=180];\n";
                }
                else if( is_ff( (Gate *) ptr ) )
                {
                    dot_fd << "  " << std::to_string( ( (Gate *) ptr )->get_id() )
                           << ( coords.size() == 0 ? ";\n" : " [" + coords + "];\n" );
                }
            }

            std::queue<std::pair<igraph_integer_t, std::string>> queue;
            for( const igraph_integer_t &root : m_roots )
            {
                queue.push( { root, "blue" } );
            }

            igraph_error_t ierror;
            std::unordered_set<igraph_integer_t> visited;
            while( !queue.empty() )
            {
                const std::pair<igraph_integer_t, std::string> pair = queue.front();
                queue.pop();

                const igraph_integer_t vertex = pair.first;
                std::string edge_color = pair.second;

                if( visited.find( vertex ) != visited.end() )
                {
                    continue;
                }

                visited.insert( vertex );

                const void *sptr = m_vertices_to_ptrs.at( vertex );
                const std::string stype = m_ptrs_to_types.at( sptr );

                if( stype == "gate" && is_inverter( (Gate *) sptr ) )
                {
                    edge_color = edge_color == "red" ? "blue" : "red";
                }

                igraph_vector_int_t neighbors;
                if( ( ierror = igraph_vector_int_init( &neighbors, 0 ) ) != IGRAPH_SUCCESS )
                {
                    dot_fd.close();
                    return ERR( igraph_strerror( ierror ) );
                }

                if( ( ierror = igraph_neighbors( m_igraph_ptr, &neighbors, vertex, IGRAPH_OUT ) ) != IGRAPH_SUCCESS )
                {
                    dot_fd.close();
                    igraph_vector_int_destroy( &neighbors );
                    return ERR( igraph_strerror( ierror ) );
                }

                for( igraph_integer_t idx = 0; idx < igraph_vector_int_size( &neighbors ); idx++ )
                {
                    const std::string src_id =
                        stype == "gate" ? std::to_string( ( (Gate *) sptr )->get_id() ) : ( (Net *) sptr )->get_name();

                    const void *dptr = m_vertices_to_ptrs.at( VECTOR( neighbors )[idx] );
                    const std::string dtype = m_ptrs_to_types.at( dptr );
                    const std::string dst_id =
                        dtype == "gate" ? std::to_string( ( (Gate *) dptr )->get_id() ) : ( (Net *) dptr )->get_name();

                    dot_fd << "  " << src_id << " -> " << dst_id << " [color=" << edge_color << "];\n";
                    queue.push( { VECTOR( neighbors )[idx], edge_color } );
                }

                igraph_vector_int_destroy( &neighbors );
            }

            dot_fd << "}\n";
            dot_fd.close();

            return OK( {} );
        }

        const Netlist *ClockTree::get_netlist() const
        {
            return m_netlist;
        }

        const igraph_t *ClockTree::get_igraph() const
        {
            return m_igraph_ptr;
        }
    }  // namespace cte
}  // namespace hal

// BUG: looks like ~20 vertices plus their edges are missing in benchmark
// TODO: investigate possible BUG

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

            igraph_error_t
            in_callback( const igraph_t *graph, igraph_integer_t vid, igraph_integer_t dist, void *extra )
            {
                return igraph_vector_int_push_back( (igraph_vector_int_t *) extra, vid );
            }
        }  // namespace

        ClockTree::ClockTree( const Netlist *netlist )
            : m_netlist( netlist )
            , m_igraph_ptr( &m_igraph )
        {
        }

        ClockTree::ClockTree( const Netlist *netlist,
                              igraph_t &&igraph,
                              std::unordered_set<igraph_integer_t> &&roots,
                              std::unordered_map<igraph_integer_t, const void *> &&vertices_to_ptrs,
                              std::unordered_map<const void *, PtrType> &&ptrs_to_types )
            : m_netlist( netlist )
            , m_igraph( std::move( igraph ) )
            , m_roots( std::move( roots ) )
            , m_vertices_to_ptrs( std::move( vertices_to_ptrs ) )
            , m_ptrs_to_types( std::move( ptrs_to_types ) )
        {
            m_igraph_ptr = &m_igraph;

            for( const auto &[vertex, ptr] : m_vertices_to_ptrs )
            {
                m_ptrs_to_vertices[ptr] = vertex;
            }
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
            std::unordered_map<const void *, PtrType> ptrs_to_type;

            std::queue<std::tuple<const Gate *, const Gate *, std::vector<const Gate *>>> queue;
            NetlistTraversalDecorator ntd = NetlistTraversalDecorator( *netlist );
            std::unordered_set<std::pair<const Gate *, const Gate *>, VoidPtrHash> visited;

            std::unordered_map<std::pair<const void *, const void *>, std::vector<const Gate *>, VoidPtrHash, PairPtrEq>
                paths;

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
                    ptrs_to_type[(void *) clk] = PtrType::NET;
                    continue;
                }
                else if( clk->get_num_of_sources() == 0 )
                {
                    log_warning( "clock_tree_extractor",
                                 "unrouted clock net with ID {} ignored",
                                 std::to_string( clk->get_id() ) );
                    continue;
                }

                queue.push( { ff, clk->get_sources().front()->get_gate(), std::vector<const Gate *>{} } );

                vertices.insert( (void *) ff );
                ptrs_to_type[(void *) ff] = PtrType::GATE;
            }

            const std::unordered_set<const Gate *> toggle_ffs = get_toggle_ffs( netlist );

            while( !queue.empty() )
            {
                const std::tuple<const Gate *, const Gate *, std::vector<const Gate *>> tuple = queue.front();
                queue.pop();

                Gate *source = (Gate *) std::get<1>( tuple );
                Gate *reference = (Gate *) std::get<0>( tuple );
                std::vector<const Gate *> path = std::get<2>( tuple );

                path.push_back( source );

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

                    ptrs_to_type[(void *) source] = PtrType::GATE;
                    ptrs_to_type[(void *) reference] = PtrType::GATE;

                    edges.insert( { (void *) source, (void *) reference } );
                    path.push_back( reference );
                    paths[{ (void *) source, (void *) reference }] = path;
                    path.clear();

                    if( is_ff( source ) )
                    {
                        continue;
                    }

                    reference = (Gate *) source;
                }

                visited.insert( std::make_pair( reference, source ) );

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

                        ptrs_to_type[(void *) net] = PtrType::NET;
                        ptrs_to_type[(void *) reference] = PtrType::GATE;

                        edges.insert( { (void *) net, (void *) reference } );

                        // paths[{ (void *) net, (void *) reference }] = path;
                        // path.clear();

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
                        queue.push( { reference, new_source, path } );
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
            clock_tree->m_paths = std::move( paths );

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
                if( m_ptrs_to_types.at( ptr ) == PtrType::NET )
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
                const PtrType stype = m_ptrs_to_types.at( sptr );

                if( stype == PtrType::GATE && is_inverter( (Gate *) sptr ) )
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
                    const std::string src_id = stype == PtrType::GATE ? std::to_string( ( (Gate *) sptr )->get_id() )
                                                                      : ( (Net *) sptr )->get_name();

                    const void *dptr = m_vertices_to_ptrs.at( VECTOR( neighbors )[idx] );
                    const PtrType dtype = m_ptrs_to_types.at( dptr );
                    const std::string dst_id = dtype == PtrType::GATE ? std::to_string( ( (Gate *) dptr )->get_id() )
                                                                      : ( (Net *) dptr )->get_name();

                    dot_fd << "  " << src_id << " -> " << dst_id << " [color=" << edge_color << "];\n";
                    queue.push( { VECTOR( neighbors )[idx], edge_color } );
                }

                igraph_vector_int_destroy( &neighbors );
            }

            dot_fd << "}\n";
            dot_fd.close();

            return OK( {} );
        }

        Result<std::unique_ptr<ClockTree>> ClockTree::get_subtree( const void *ptr, const bool parent ) const
        {
            auto it = m_ptrs_to_vertices.find( ptr );
            if( it == m_ptrs_to_vertices.end() )
            {
                return ERR( "object is not part of clock tree" );
            }

            igraph_error_t ierror;
            igraph_integer_t root = it->second;
            if( parent )
            {
                igraph_vector_int_t parents;
                if( ( ierror = igraph_vector_int_init( &parents, 0 ) ) != IGRAPH_SUCCESS )
                {
                    return ERR( igraph_strerror( ierror ) );
                }

                if( ( ierror = igraph_neighbors( m_igraph_ptr, &parents, root, IGRAPH_IN ) ) != IGRAPH_SUCCESS )
                {
                    igraph_vector_int_destroy( &parents );
                    return ERR( igraph_strerror( ierror ) );
                }

                // Only accept, if there is only one parent vertex for now.
                if( igraph_vector_int_size( &parents ) == 1 )
                {
                    root = VECTOR( parents )[0];
                }

                igraph_vector_int_destroy( &parents );
            }

            igraph_vector_int_t vertices;
            if( ( ierror = igraph_vector_int_init( &vertices, 0 ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            if( ( ierror = igraph_dfs( m_igraph_ptr,
                                       root,
                                       IGRAPH_OUT,
                                       false,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       in_callback,
                                       nullptr,
                                       &vertices ) )
                != IGRAPH_SUCCESS )
            {
                igraph_vector_int_destroy( &vertices );
                return ERR( igraph_strerror( ierror ) );
            }

            igraph_vs_t vs;
            if( ( ierror = igraph_vs_vector( &vs, &vertices ) ) != IGRAPH_SUCCESS )
            {
                igraph_vector_int_destroy( &vertices );
                return ERR( igraph_strerror( ierror ) );
            }

            igraph_vector_int_t map;
            if( ( ierror = igraph_vector_int_init( &map, igraph_vcount( m_igraph_ptr ) ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            igraph_t igraph;
            if( ( ierror =
                      igraph_induced_subgraph_map( m_igraph_ptr, &igraph, vs, IGRAPH_SUBGRAPH_AUTO, &map, nullptr ) )
                != IGRAPH_SUCCESS )
            {
                igraph_vs_destroy( &vs );
                igraph_vector_int_destroy( &map );
                igraph_vector_int_destroy( &vertices );
                return ERR( igraph_strerror( ierror ) );
            }

            igraph_vs_destroy( &vs );
            igraph_vector_int_destroy( &vertices );

            std::unordered_set<igraph_integer_t> roots;
            std::unordered_map<const void *, PtrType> ptrs_to_types;
            std::unordered_map<igraph_integer_t, const void *> vertices_to_ptrs;

            for( igraph_integer_t idx = 0; idx < igraph_vector_int_size( &map ); idx++ )
            {
                const igraph_integer_t vertex = VECTOR( map )[idx];
                if( vertex == 0 )
                {
                    continue;
                }

                const void *ptr = m_vertices_to_ptrs.at( idx );

                vertices_to_ptrs[vertex - 1] = ptr;
                ptrs_to_types[ptr] = m_ptrs_to_types.at( ptr );
            }

            igraph_vector_int_destroy( &map );

            igraph_vector_int_t indegrees;
            if( ( ierror = igraph_vector_int_init( &indegrees, igraph_vcount( &igraph ) ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            if( ( ierror = igraph_degree( &igraph, &indegrees, igraph_vss_all(), IGRAPH_IN, IGRAPH_NO_LOOPS ) )
                != IGRAPH_SUCCESS )
            {
                igraph_vector_int_destroy( &indegrees );
                return ERR( igraph_strerror( ierror ) );
            }

            for( igraph_integer_t idx = 0; idx < igraph_vector_int_size( &indegrees ); idx++ )
            {
                if( VECTOR( indegrees )[idx] != 0 )
                {
                    continue;
                }
                roots.insert( idx );
            }

            igraph_vector_int_destroy( &indegrees );

            return OK( std::make_unique<ClockTree>( m_netlist,
                                                    std::move( igraph ),
                                                    std::move( roots ),
                                                    std::move( vertices_to_ptrs ),
                                                    std::move( ptrs_to_types ) ) );
        }

        Result<igraph_integer_t> ClockTree::get_vertex_from_ptr( const void *ptr ) const
        {
            auto it = m_ptrs_to_vertices.find( ptr );
            if( it == m_ptrs_to_vertices.end() )
            {
                return ERR( "object is not part of clock tree" );
            }

            return OK( it->second );
        }

        Result<std::pair<const void *, PtrType>> ClockTree::get_ptr_from_vertex( const igraph_integer_t vertex ) const
        {
            auto it = m_vertices_to_ptrs.find( vertex );
            if( it == m_vertices_to_ptrs.end() )
            {
                return ERR( "object is not part of clock tree" );
            }

            return OK( std::make_pair( it->second, m_ptrs_to_types.at( it->second ) ) );
        }

        Result<std::vector<igraph_integer_t>>
        ClockTree::get_vertices_from_ptrs( const std::vector<const void *> &ptrs ) const
        {
            std::vector<igraph_integer_t> result;

            for( const void *ptr : ptrs )
            {
                auto res = get_vertex_from_ptr( ptr );
                if( res.is_error() )
                {
                    return ERR( res.get_error().get() );
                }

                result.push_back( res.get() );
            }

            return OK( result );
        }

        Result<std::vector<std::pair<const void *, PtrType>>>
        ClockTree::get_ptrs_from_vertices( const std::vector<igraph_integer_t> &vertices ) const
        {
            std::vector<std::pair<const void *, PtrType>> result;

            for( const igraph_integer_t vertex : vertices )
            {
                auto res = get_ptr_from_vertex( vertex );
                if( res.is_error() )
                {
                    return ERR( res.get_error().get() );
                }

                result.push_back( res.get() );
            }

            return OK( result );
        }

        const std::vector<const Gate *> ClockTree::get_gates() const
        {
            std::vector<const Gate *> result;

            for( const auto &[ptr, type] : m_ptrs_to_types )
            {
                if( type == PtrType::GATE )
                {
                    result.push_back( (const Gate *) ptr );
                }
            }

            return result;
        }

        const std::vector<const Net *> ClockTree::get_nets() const
        {
            std::vector<const Net *> result;

            for( const auto &[ptr, type] : m_ptrs_to_types )
            {
                if( type == PtrType::NET )
                {
                    result.push_back( (const Net *) ptr );
                }
            }

            return result;
        }

        const std::unordered_map<const void *, PtrType> ClockTree::get_all() const
        {
            return m_ptrs_to_types;
        }

        const Netlist *ClockTree::get_netlist() const
        {
            return m_netlist;
        }

        const igraph_t *ClockTree::get_igraph() const
        {
            return m_igraph_ptr;
        }

        const std::
            unordered_map<std::pair<const void *, const void *>, std::vector<const Gate *>, VoidPtrHash, PairPtrEq>
            ClockTree::get_paths() const
        {
            return m_paths;
        }

        Result<std::vector<std::pair<const void *, PtrType>>>
        ClockTree::get_neighbors( const void *ptr, igraph_neimode_t direction ) const
        {
            auto it = m_ptrs_to_vertices.find( ptr );
            if( it == m_ptrs_to_vertices.end() )
            {
                return ERR( "object is not part of clock tree" );
            }

            igraph_error_t ierror;
            igraph_vector_int_t neighbors;

            if( ( ierror = igraph_vector_int_init( &neighbors, 0 ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            if( ( ierror = igraph_neighbors( m_igraph_ptr, &neighbors, it->second, direction ) ) != IGRAPH_SUCCESS )
            {
                igraph_vector_int_destroy( &neighbors );
                return ERR( igraph_strerror( ierror ) );
            }

            std::vector<std::pair<const void *, PtrType>> result;
            for( igraph_integer_t idx = 0; idx < igraph_vector_int_size( &neighbors ); idx++ )
            {
                const void *n_ptr = m_vertices_to_ptrs.at( VECTOR( neighbors )[idx] );
                result.push_back( std::make_pair( n_ptr, m_ptrs_to_types.at( n_ptr ) ) );
            }

            igraph_vector_int_destroy( &neighbors );

            return OK( result );
        }
    }  // namespace cte
}  // namespace hal

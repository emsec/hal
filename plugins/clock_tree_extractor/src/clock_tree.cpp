// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "clock_tree_extractor/clock_tree.h"

#include "clock_tree_extractor/utils.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/result.h"

#include <algorithm>
#include <cstddef>
#include <igraph/igraph.h>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hal::cte
{
    namespace
    {
        std::unordered_set<const Gate *> get_toggle_ffs( const Netlist *netlist )
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

        igraph_error_t in_callback( const igraph_t *graph, igraph_int_t vid, igraph_int_t dist, void *extra )
        {
            return igraph_vector_int_push_back( static_cast<igraph_vector_int_t *>( extra ), vid );
        }

        /**
             * Replaces igraph's default error handler, which aborts the whole process, with one that logs and returns,
             * so that a failing igraph call hands its error code back to the caller and ends up in a Result instead.
             * The previous handler is restored when the guard goes out of scope, so other plugins using igraph keep
             * whatever handler they rely on. Guards nest, e.g., recover_branches calling get_neighbors.
             *
             * Put one at the start of every function that calls into igraph.
             */
        class ScopedIgraphErrorHandler
        {
          public:
            ScopedIgraphErrorHandler()
                : m_previous( igraph_set_error_handler( &handle_error ) )
            {
            }

            ~ScopedIgraphErrorHandler()
            {
                igraph_set_error_handler( m_previous );
            }

            ScopedIgraphErrorHandler( const ScopedIgraphErrorHandler & ) = delete;
            ScopedIgraphErrorHandler &operator=( const ScopedIgraphErrorHandler & ) = delete;

          private:
            static void handle_error( const char *reason, const char *file, int line, igraph_error_t igraph_errno )
            {
                // igraph calls the handler once per level an error passes through, so this may log several times
                // per error. log first: IGRAPH_FINALLY_FREE may invalidate reason
                log_debug( "clock_tree_extractor",
                           "igraph error at {}:{}: {} ({})",
                           file,
                           line,
                           reason,
                           igraph_strerror( igraph_errno ) );

                // required of every handler that does not abort: frees igraph's temporary allocations
                IGRAPH_FINALLY_FREE();
            }

            igraph_error_handler_t *m_previous;
        };
    }  // namespace

    ClockTree::ClockTree( const Netlist *netlist,
                          igraph_t &&igraph,
                          std::unordered_set<igraph_int_t> &&roots,
                          std::unordered_map<igraph_int_t, const void *> &&vertices_to_ptrs,
                          std::unordered_map<const void *, PtrType> &&ptrs_to_types )
        : m_netlist( netlist )
        , m_igraph( igraph )
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

        std::vector<Diagnostic> diagnostics;
        const auto report = [&diagnostics]( Severity severity,
                                            DiagnosticCode code,
                                            PtrType type,
                                            const void *ptr,
                                            u32 id,
                                            std::string message ) {
            if( severity == Severity::error )
            {
                log_error( "clock_tree_extractor", "{}", message );
            }
            else
            {
                log_warning( "clock_tree_extractor", "{}", message );
            }
            diagnostics.push_back( { severity, code, type, ptr, id, std::move( message ) } );
        };

        std::unordered_set<const void *> vertices;
        std::unordered_set<std::pair<const void *, const void *>, VoidPtrHash> edges;
        std::unordered_map<const void *, PtrType> ptrs_to_type;

        std::queue<std::tuple<const Gate *, const Gate *, std::vector<const Gate *>>> queue;
        std::unordered_set<std::pair<const Gate *, const Gate *>, VoidPtrHash> visited;
        std::unordered_set<const Gate *> expanded;

        for( const Gate *ff : netlist->get_gates( is_ff ) )
        {
            vertices.insert( ff );
            ptrs_to_type[ff] = GATE;

            const std::vector<GatePin *> clock_pins = ff->get_type()->get_pins( []( const auto &p ) {
                return ( p->get_direction() == PinDirection::input ) && ( p->get_type() == PinType::clock );
            } );

            if( clock_pins.size() != 1 )
            {
                report( Severity::error,
                        DiagnosticCode::invalid_clock_pin_count,
                        GATE,
                        ff,
                        ff->get_id(),
                        "invalid number of input clock pins at gate '" + ff->get_name() + "' with ID "
                            + std::to_string( ff->get_id() ) );
                continue;
            }

            const Net *clk = ff->get_fan_in_net( clock_pins.front() );
            if( clk == nullptr )
            {
                report( Severity::error,
                        DiagnosticCode::unconnected_clock_pin,
                        GATE,
                        ff,
                        ff->get_id(),
                        "no net connected to clock pin at gate '" + ff->get_name() + "' with ID "
                            + std::to_string( ff->get_id() ) );
                continue;
            }

            if( clk->get_num_of_sources() > 1 )
            {
                bool valid = true;
                for( const Endpoint *source_ep : clk->get_sources() )
                {
                    const Gate *gate = source_ep->get_gate();
                    if( !( is_buffer( gate ) || is_inverter( gate ) ) )
                    {
                        // In theory, it should be either all buffers or all inverters. But depending on
                        // extraction results, e.g., it could happen that a buffer is split into two inverters.
                        // So I just assume it is fine if all the sources are either a buffer or an inverter
                        // without enforcing strict buffer only or inverter only.
                        valid = false;
                        break;
                    }
                }
                if( !valid )
                {
                    report( Severity::error,
                            DiagnosticCode::invalid_clock_net_sources,
                            NET,
                            clk,
                            clk->get_id(),
                            "invalid number of sources for clock net with ID " + std::to_string( clk->get_id() ) );
                    continue;
                }
            }
            else if( clk->is_global_input_net() )
            {
                // the flip-flop is clocked straight from the outside: the net is the root and the flip-flop its only child
                vertices.insert( clk );
                ptrs_to_type[clk] = NET;
                edges.insert( { clk, ff } );
                continue;
            }
            else if( clk->get_num_of_sources() == 0 )
            {
                report( Severity::warning,
                        DiagnosticCode::unrouted_clock_net,
                        NET,
                        clk,
                        clk->get_id(),
                        "unrouted clock net with ID " + std::to_string( clk->get_id() ) + " ignored" );
                continue;
            }

            for( const Endpoint *source_ep : clk->get_sources() )
            {
                const Gate *gate = source_ep->get_gate();
                queue.emplace( ff, gate, std::vector<const Gate *>{ ff } );
            }
        }

        const std::unordered_set<const Gate *> toggle_ffs = get_toggle_ffs( netlist );

        while( !queue.empty() )
        {
            const Gate *reference = std::get<0>( queue.front() );
            const Gate *source = std::get<1>( queue.front() );
            std::vector<const Gate *> path = std::move( std::get<2>( queue.front() ) );
            queue.pop();

            path.push_back( source );

            if( is_latch( source ) )
            {
                // Ignore latches
                continue;
            }
            else if( is_buffer( source ) || is_inverter( source ) || is_delay( source ) || is_ff( source ) )
            {
                if( is_ff( source ) && toggle_ffs.find( source ) == toggle_ffs.end() )
                {
                    // Include only toggle flip-flops for now
                    continue;
                }

                for( const Gate *gate : path )
                {
                    vertices.insert( gate );
                    ptrs_to_type[gate] = GATE;
                }

                for( std::size_t idx = 0; idx < path.size() - 1; idx++ )
                {
                    edges.insert( { path[idx + 1], path[idx] } );
                }

                path.clear();
                path.push_back( source );

                if( is_ff( source ) )
                {
                    continue;
                }

                // prevent runtime explosion
                if( !expanded.insert( source ).second )
                {
                    // the path is merged now, and the fan-in of this gate has already been walked
                    continue;
                }

                reference = source;
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
                    for( const Gate *gate : path )
                    {
                        vertices.insert( gate );
                        ptrs_to_type[gate] = GATE;
                    }

                    for( std::size_t idx = 0; idx < path.size() - 1; idx++ )
                    {
                        edges.insert( { path[idx + 1], path[idx] } );
                    }

                    vertices.insert( net );
                    ptrs_to_type[net] = NET;

                    edges.insert( { net, path.back() } );

                    path.clear();
                    path.push_back( source );

                    continue;
                }

                if( net->get_num_of_sources() == 0 )
                {
                    report( Severity::warning,
                            DiagnosticCode::unrouted_clock_net,
                            NET,
                            net,
                            net->get_id(),
                            "unrouted clock net with ID " + std::to_string( net->get_id() ) + " ignored" );
                    continue;
                }
                if( net->get_num_of_sources() > 1 )
                {
                    report( Severity::warning,
                            DiagnosticCode::multi_driven_clock_net,
                            NET,
                            net,
                            net->get_id(),
                            "multi-driven clock net with ID " + std::to_string( net->get_id() ) + " ignored" );
                    continue;
                }

                const Gate *new_source = net->get_sources().front()->get_gate();
                if( visited.find( { reference, new_source } ) == visited.end() )
                {
                    queue.emplace( reference, new_source, path );
                }
            }
        }

        const ScopedIgraphErrorHandler igraph_error_handler;

        // everything is built in locals first and only moved into a ClockTree once it all succeeded, so an
        // early return never destroys a ClockTree whose graph was not initialized
        std::unordered_map<igraph_int_t, const void *> vertices_to_ptrs;
        std::unordered_map<const void *, igraph_int_t> ptrs_to_vertices;

        igraph_int_t idx = 0;
        for( const void *vertex : vertices )
        {
            const igraph_int_t vertex_id = idx++;

            vertices_to_ptrs[vertex_id] = vertex;
            ptrs_to_vertices[vertex] = vertex_id;
        }

        igraph_error_t ierror;
        igraph_vector_int_t iedges;
        if( ( ierror = igraph_vector_int_init( &iedges, 2 * edges.size() ) ) != IGRAPH_SUCCESS )
        {
            return ERR( igraph_strerror( ierror ) );
        }

        idx = 0;
        for( const auto &[src, dst] : edges )
        {
            VECTOR( iedges )[idx++] = ptrs_to_vertices.at( src );
            VECTOR( iedges )[idx++] = ptrs_to_vertices.at( dst );
        }

        igraph_t igraph;
        if( ( ierror = igraph_create( &igraph, &iedges, vertices.size(), IGRAPH_DIRECTED ) ) != IGRAPH_SUCCESS )
        {
            igraph_vector_int_destroy( &iedges );
            return ERR( igraph_strerror( ierror ) );
        }

        igraph_vector_int_destroy( &iedges );

        igraph_vector_int_t indegrees;
        if( ( ierror = igraph_vector_int_init( &indegrees, 0 ) ) != IGRAPH_SUCCESS )
        {
            igraph_destroy( &igraph );
            return ERR( igraph_strerror( ierror ) );
        }

        if( ( ierror = igraph_degree( &igraph, &indegrees, igraph_vss_all(), IGRAPH_IN, IGRAPH_NO_LOOPS ) )
            != IGRAPH_SUCCESS )
        {
            igraph_destroy( &igraph );
            igraph_vector_int_destroy( &indegrees );
            return ERR( igraph_strerror( ierror ) );
        }

        std::unordered_set<igraph_int_t> roots;
        for( idx = 0; idx < igraph_vector_int_size( &indegrees ); idx++ )
        {
            if( VECTOR( indegrees )[idx] != 0 )
            {
                continue;
            }
            roots.insert( idx );
        }

        igraph_vector_int_destroy( &indegrees );

        // the ClockTree takes ownership of igraph from here on and destroys it in its destructor.
        // plain new instead of make_unique, since the constructor is private
        auto clock_tree = std::unique_ptr<ClockTree>( new ClockTree( netlist,
                                                                     std::move( igraph ),
                                                                     std::move( roots ),
                                                                     std::move( vertices_to_ptrs ),
                                                                     std::move( ptrs_to_type ) ) );
        clock_tree->m_diagnostics = std::move( diagnostics );

        return OK( std::move( clock_tree ) );
    }

    Result<std::unique_ptr<ClockTree>> ClockTree::get_subtree( const void *ptr, const bool parent ) const
    {
        const ScopedIgraphErrorHandler igraph_error_handler;

        auto it = m_ptrs_to_vertices.find( ptr );
        if( it == m_ptrs_to_vertices.end() )
        {
            return ERR( "object is not part of clock tree" );
        }

        igraph_error_t ierror;
        igraph_int_t root = it->second;
        if( parent )
        {
            igraph_vector_int_t parents;
            if( ( ierror = igraph_vector_int_init( &parents, 0 ) ) != IGRAPH_SUCCESS )
            {
                return ERR( igraph_strerror( ierror ) );
            }

            if( ( ierror =
                      igraph_neighbors( m_igraph_ptr, &parents, root, IGRAPH_IN, IGRAPH_NO_LOOPS, IGRAPH_NO_MULTIPLE ) )
                != IGRAPH_SUCCESS )
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
            igraph_vs_destroy( &vs );
            igraph_vector_int_destroy( &vertices );
            return ERR( igraph_strerror( ierror ) );
        }

        igraph_t igraph;
        if( ( ierror = igraph_induced_subgraph_map( m_igraph_ptr, &igraph, vs, IGRAPH_SUBGRAPH_AUTO, &map, nullptr ) )
            != IGRAPH_SUCCESS )
        {
            igraph_vs_destroy( &vs );
            igraph_vector_int_destroy( &map );
            igraph_vector_int_destroy( &vertices );
            return ERR( igraph_strerror( ierror ) );
        }

        igraph_vs_destroy( &vs );
        igraph_vector_int_destroy( &vertices );

        std::unordered_set<igraph_int_t> roots;
        std::unordered_map<const void *, PtrType> ptrs_to_types;
        std::unordered_map<igraph_int_t, const void *> vertices_to_ptrs;

        for( igraph_int_t idx = 0; idx < igraph_vector_int_size( &map ); idx++ )
        {
            const igraph_int_t vertex = VECTOR( map )[idx];
            if( vertex == -1 )
            {
                continue;
            }

            const void *ptr = m_vertices_to_ptrs.at( idx );

            vertices_to_ptrs[vertex] = ptr;
            ptrs_to_types[ptr] = m_ptrs_to_types.at( ptr );
        }

        igraph_vector_int_destroy( &map );

        igraph_vector_int_t indegrees;
        if( ( ierror = igraph_vector_int_init( &indegrees, igraph_vcount( &igraph ) ) ) != IGRAPH_SUCCESS )
        {
            igraph_destroy( &igraph );
            return ERR( igraph_strerror( ierror ) );
        }

        if( ( ierror = igraph_degree( &igraph, &indegrees, igraph_vss_all(), IGRAPH_IN, IGRAPH_NO_LOOPS ) )
            != IGRAPH_SUCCESS )
        {
            igraph_destroy( &igraph );
            igraph_vector_int_destroy( &indegrees );
            return ERR( igraph_strerror( ierror ) );
        }

        for( igraph_int_t idx = 0; idx < igraph_vector_int_size( &indegrees ); idx++ )
        {
            if( VECTOR( indegrees )[idx] != 0 )
            {
                continue;
            }
            roots.insert( idx );
        }

        igraph_vector_int_destroy( &indegrees );

        return OK( std::unique_ptr<ClockTree>( new ClockTree( m_netlist,
                                                              std::move( igraph ),
                                                              std::move( roots ),
                                                              std::move( vertices_to_ptrs ),
                                                              std::move( ptrs_to_types ) ) ) );
    }

    Result<igraph_int_t> ClockTree::get_vertex_from_ptr( const void *ptr ) const
    {
        auto it = m_ptrs_to_vertices.find( ptr );
        if( it == m_ptrs_to_vertices.end() )
        {
            return ERR( "object is not part of clock tree" );
        }

        return OK( it->second );
    }

    Result<std::pair<const void *, PtrType>> ClockTree::get_ptr_from_vertex( const igraph_int_t vertex ) const
    {
        auto it = m_vertices_to_ptrs.find( vertex );
        if( it == m_vertices_to_ptrs.end() )
        {
            return ERR( "object is not part of clock tree" );
        }

        return OK( std::make_pair( it->second, m_ptrs_to_types.at( it->second ) ) );
    }

    Result<std::vector<igraph_int_t>> ClockTree::get_vertices_from_ptrs( const std::vector<const void *> &ptrs ) const
    {
        std::vector<igraph_int_t> result;

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
    ClockTree::get_ptrs_from_vertices( const std::vector<igraph_int_t> &vertices ) const
    {
        std::vector<std::pair<const void *, PtrType>> result;

        for( const igraph_int_t vertex : vertices )
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

    std::vector<const Gate *> ClockTree::get_gates() const
    {
        std::vector<const Gate *> result;

        for( const auto &[ptr, type] : m_ptrs_to_types )
        {
            if( type == PtrType::GATE )
            {
                result.push_back( static_cast<const Gate *>( ptr ) );
            }
        }

        return result;
    }

    std::vector<const Net *> ClockTree::get_nets() const
    {
        std::vector<const Net *> result;

        for( const auto &[ptr, type] : m_ptrs_to_types )
        {
            if( type == PtrType::NET )
            {
                result.push_back( static_cast<const Net *>( ptr ) );
            }
        }

        return result;
    }

    const std::unordered_map<const void *, PtrType> &ClockTree::get_all() const
    {
        return m_ptrs_to_types;
    }

    const Netlist *ClockTree::get_netlist() const
    {
        return m_netlist;
    }

    const std::vector<Diagnostic> &ClockTree::get_diagnostics() const
    {
        return m_diagnostics;
    }

    const std::unordered_set<igraph_int_t> &ClockTree::get_roots() const
    {
        return m_roots;
    }

    const igraph_t *ClockTree::get_igraph() const
    {
        return m_igraph_ptr;
    }

    Result<std::vector<std::pair<const void *, PtrType>>>
    ClockTree::get_neighbors( const void *ptr, const igraph_neimode_t direction ) const
    {
        const ScopedIgraphErrorHandler igraph_error_handler;

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

        if( ( ierror = igraph_neighbors(
                  m_igraph_ptr, &neighbors, it->second, direction, IGRAPH_NO_LOOPS, IGRAPH_NO_MULTIPLE ) )
            != IGRAPH_SUCCESS )
        {
            igraph_vector_int_destroy( &neighbors );
            return ERR( igraph_strerror( ierror ) );
        }

        std::vector<std::pair<const void *, PtrType>> result;
        for( igraph_int_t idx = 0; idx < igraph_vector_int_size( &neighbors ); idx++ )
        {
            const void *n_ptr = m_vertices_to_ptrs.at( VECTOR( neighbors )[idx] );
            result.emplace_back( n_ptr, m_ptrs_to_types.at( n_ptr ) );
        }

        igraph_vector_int_destroy( &neighbors );

        return OK( result );
    }
}  // namespace hal::cte

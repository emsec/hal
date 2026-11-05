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
#include <unordered_map>
#include <unordered_set>

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
                                            std::string message,
                                            const Endpoint *endpoint = nullptr ) {
            if( severity == Severity::error )
            {
                log_error( "clock_tree_extractor", "{}", message );
            }
            else if( severity == Severity::warning )
            {
                log_warning( "clock_tree_extractor", "{}", message );
            }
            else if( severity == Severity::info )
            {
                log_info( "clock_tree_extractor", "{}", message );
            }
            else
            {
                log_debug( "clock_tree_extractor", "{}", message );
            }
            diagnostics.push_back( { severity, code, type, ptr, id, std::move( message ), endpoint } );
        };

        // new
        std::queue<const Net *> nqueue;
        std::unordered_set<const Net *> nvisited;
        std::unordered_set<const Net *> deemed_clock_net;

        for( const Gate *ff : netlist->get_gates( is_ff ) )
        {
            const std::vector<GatePin *> clock_pins = ff->get_type()->get_pins( []( const auto &p ) {
                return ( p->get_direction() == PinDirection::input ) && ( p->get_type() == PinType::clock );
            } );

            /*
                if( clock_pins.size() != 1 )
                {
                    report( Severity::error,
                            DiagnosticCode::invalid_clock_pin_count,
                            PtrType::GATE,
                            ff,
                            ff->get_id(),
                            "invalid number of input clock pins at gate '" + ff->get_name() + "' with ID "
                                + std::to_string( ff->get_id() ) );
                    continue;
                }
                */

            for( const auto &pin : clock_pins )
            {
                const Net *clk = ff->get_fan_in_net( pin );

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

                nqueue.emplace( clk );

                // improve (also log corresponding gate)
                report( Severity::debug,
                        DiagnosticCode::enqueued_net,
                        NET,
                        clk,
                        clk->get_id(),
                        "enqueued net '" + clk->get_name() + "' with ID " + std::to_string( clk->get_id() ) );
            }
        }

        while( !nqueue.empty() )
        {
            const Net *net = nqueue.front();
            nqueue.pop();

            if( !nvisited.insert( net ).second )
            {
                // prevent double processing of net
                continue;
            }

            if( net->is_global_input_net() )
            {
                // no sources for global input
                // traversal is at end
                deemed_clock_net.emplace( net );
                continue;
            }

            if( net->get_num_of_sources() == 0 )
            {
                report( Severity::warning,
                        DiagnosticCode::unrouted_net,
                        NET,
                        net,
                        net->get_id(),
                        "unrouted net with ID " + std::to_string( net->get_id() ) + " ignored" );
                continue;
            }

            if( net->get_num_of_sources() > 1 )
            {
                // open question: how to support meshes?
                report( Severity::warning,
                        DiagnosticCode::multi_driven_net,
                        NET,
                        net,
                        net->get_id(),
                        "multi-driven net with ID " + std::to_string( net->get_id() ) + " ignored" );
                continue;
            }

            deemed_clock_net.emplace( net );

            const Gate *source = net->get_sources().front()->get_gate();

            if( source->get_fan_in_endpoints().size() > 1 )
            {
                if( is_ff( source ) )
                {
                    // usually, registers don't feed the clock
                    continue;
                }

                if( is_latch( source ) )
                {
                    // idk yet how to include these
                    continue;
                }

                if( is_mux( source ) )
                {
                    const std::vector<GatePin *> data_pins = source->get_type()->get_pins( []( const auto &p ) {
                        return ( p->get_direction() == PinDirection::input ) && ( p->get_type() == PinType::data );
                    } );

                    for( const auto &pin : data_pins )
                    {
                        const Net *input = source->get_fan_in_net( pin );

                        if( input == nullptr )
                        {
                            continue;
                        }

                        // TODO: create own function to avoid duplicate code
                        if( input->is_gnd_net() || input->is_vcc_net() )
                        {
                            continue;
                        }

                        nqueue.emplace( input );
                    }
                    continue;
                }

                if( is_oscillator( source ) || is_pll( source ) )
                {
                    // could be a valid clock source
                    // nothing to do, since output is already deemed clock net
                    continue;
                }

                // TODO: clock gating

                report( Severity::warning,
                        DiagnosticCode::multi_input_gate,
                        GATE,
                        source,
                        source->get_id(),
                        "multi-input gate with ID " + std::to_string( source->get_id() ) + " ignored" );

                continue;
            }

            // only one source means we still have to be on the clock path
            for( const Endpoint *ep : source->get_fan_in_endpoints() )
            {
                // enqueue new candidates for clock tree interconnect if not power or ground
                if( ep->get_net()->is_gnd_net() || ep->get_net()->is_vcc_net() )
                {
                    continue;
                }

                nqueue.emplace( ep->get_net() );
            }
        }

        std::unordered_set<const void *> vertices;
        std::unordered_set<std::pair<const void *, const void *>, VoidPtrHash> edges;
        std::unordered_map<const void *, PtrType> ptrs_to_type;

        for( const Net *net : deemed_clock_net )
        {
            const void *source = net;
            if( !net->is_global_input_net() )
            {
                source = net->get_sources().front()->get_gate();
            }

            ptrs_to_type.emplace( source, ( source == net ) ? NET : GATE );
            vertices.emplace( source );

            // where the clock on this net comes from, for the diagnostics below
            const std::string clock_driver = net->is_global_input_net()
                ? std::string( "global input" )
                : "gate '" + net->get_sources().front()->get_gate()->get_name() + "'";

            for( const Endpoint *ep : net->get_destinations() )
            {
                const Gate *gate = ep->get_gate();

                if( gate->get_type()->has_property( GateTypeProperty::combinational ) )
                {
                    const std::vector<Net *> &outputs = gate->get_fan_out_nets();

                    const bool drives_clock_net =
                        std::any_of( outputs.begin(), outputs.end(), [&deemed_clock_net]( const Net *output ) {
                            return deemed_clock_net.find( output ) != deemed_clock_net.end();
                        } );

                    if( !drives_clock_net )
                    {
                        // a gate that reads a clock net belongs to the tree only if it passes the clock on, i.e.,
                        // if one of its outputs was itself deemed a clock net on the way back from a clock pin.
                        // otherwise it only consumes the clock, and keeping it would add a leaf and a branch that
                        // clock nothing.
                        // example: a clock monitor that samples the clock as data, e.g., XOR(clk, d) feeding the D
                        // pin of a flip-flop.
                        // side effect: gates whose outputs were skipped for other reasons are dropped as well,
                        // e.g., parallel buffers driving one multi-driven net, or the first of two clock gates in
                        // series, since the walk currently stops at the second one.
                        std::string output_names;
                        for( const Net *output : outputs )
                        {
                            output_names += ( output_names.empty() ? "'" : ", '" ) + output->get_name() + "'";
                        }

                        report( Severity::info,
                                DiagnosticCode::combinational_clock_sink,
                                GATE,
                                gate,
                                gate->get_id(),
                                "combinational gate '" + gate->get_name() + "' with ID "
                                    + std::to_string( gate->get_id() ) + " (" + gate->get_type()->get_name()
                                    + ") reads clock net '" + net->get_name() + "' with ID "
                                    + std::to_string( net->get_id() ) + " from " + clock_driver + " on pin '"
                                    + ep->get_pin()->get_name() + "' but drives no clock net (outputs: "
                                    + ( output_names.empty() ? std::string( "none" ) : output_names )
                                    + "), so it is not added to the clock tree",
                                ep );
                        continue;
                    }
                }

                const std::vector<GatePin *> clock_pins = gate->get_type()->get_pins( []( const auto &p ) {
                    return ( p->get_direction() == PinDirection::input ) && ( p->get_type() == PinType::clock );
                } );

                if( !clock_pins.empty()
                    && std::find( clock_pins.begin(), clock_pins.end(), ep->get_pin() ) == clock_pins.end() )
                {
                    // a cell with clock pins is only clocked through them. if a clock net reaches one of its other
                    // pins, the cell uses the clock as data or control there, and an edge would give the cell a
                    // second parent and a branch that does not clock it.
                    // examples: a divided clock that is also sampled on the D pin of a flip-flop in the fast
                    // domain, e.g., to detect its edges, or a clock that drives an asynchronous reset pin.
                    // cells without clock-typed pins are not filtered here: a latch whose enable pin is typed as
                    // enable still gets an edge when it reads a clock net on its D pin.
                    std::string clocked_by;
                    bool same_clock = false;
                    for( const GatePin *clock_pin : clock_pins )
                    {
                        const Net *clock_net = gate->get_fan_in_net( clock_pin );
                        same_clock = same_clock || clock_net == net;

                        clocked_by += ( clocked_by.empty() ? "pin '" : ", pin '" ) + clock_pin->get_name() + "' by "
                            + ( clock_net == nullptr ? std::string( "nothing" )
                                                     : "net '" + clock_net->get_name() + "' with ID "
                                        + std::to_string( clock_net->get_id() ) );
                    }

                    report( Severity::info,
                            DiagnosticCode::clock_on_non_clock_pin,
                            GATE,
                            gate,
                            gate->get_id(),
                            "clock net '" + net->get_name() + "' with ID " + std::to_string( net->get_id() ) + " from "
                                + clock_driver + " reaches pin '" + ep->get_pin()->get_name() + "' ("
                                + enum_to_string( ep->get_pin()->get_type() ) + ") of gate '" + gate->get_name()
                                + "' with ID " + std::to_string( gate->get_id() ) + " (" + gate->get_type()->get_name()
                                + "), which is clocked on " + clocked_by
                                + ( same_clock ? ", i.e., by the same net" : "" ) + ", so no edge is added",
                            ep );
                    continue;
                }

                auto destination = static_cast<const void *>( gate );

                ptrs_to_type.emplace( destination, GATE );
                vertices.emplace( destination );

                edges.emplace( source, destination );
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

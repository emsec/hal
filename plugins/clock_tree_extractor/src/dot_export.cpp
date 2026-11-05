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

#include "clock_tree_extractor/dot_export.h"

#include "clock_tree_extractor/clock_tree.h"
#include "clock_tree_extractor/utils.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"

#include <charconv>
#include <fstream>
#include <igraph/igraph.h>
#include <tuple>
#include <utility>
#include <vector>

namespace hal::cte
{
    namespace
    {
        const char *shape_of( const Gate *gate )
        {
            if( is_ff( gate ) || is_latch( gate ) )
            {
                return "box";
            }
            if( is_buffer( gate ) || is_inverter( gate ) || is_delay( gate ) )
            {
                // points in the direction of the signal, since the graph is laid out from top to bottom
                return "invtriangle";
            }
            if( is_mux( gate ) )
            {
                return "diamond";
            }
            if( is_latch( gate ) )
            {
                return "box";
            }
            if( is_oscillator( gate ) || is_pll( gate ) )
            {
                return "cylinder";
            }
            return "hexagon";
        }

        // quotes text as a DOT ID, e.g., names like a[3] or escaped Verilog identifiers
        std::string quoted( const std::string &text )
        {
            std::string result = "\"";
            for( const char character : text )
            {
                if( character == '"' || character == '\\' )
                {
                    result += '\\';
                }
                result += character;
            }
            return result + "\"";
        }

        std::optional<i32> parse_coordinate( const std::string &text )
        {
            i32 coordinate;
            const auto [end, error] = std::from_chars( text.data(), text.data() + text.size(), coordinate );
            if( error != std::errc() || end != text.data() + text.size() )
            {
                return std::nullopt;
            }
            return coordinate;
        }
    }  // namespace

    Result<std::monostate> export_dot( const ClockTree &clock_tree,
                                       const std::string &pathname,
                                       const std::optional<std::string> &x_key,
                                       const std::optional<std::string> &y_key )
    {
        if( x_key.has_value() != y_key.has_value() )
        {
            return ERR( "couldn't export clock tree to '" + pathname + "': give both coordinate keys or none" );
        }

        std::ofstream dot_file( pathname );
        if( !dot_file )
        {
            return ERR( "couldn't export clock tree to '" + pathname + "'" );
        }

        dot_file << "digraph {\n"
                 << "  comment=\"created by HAL plugin clock_tree_extractor\"\n"
                 << "  node [color=black fontcolor=black]\n"
                 << "  edge [color=black]\n";

        const igraph_t *graph = clock_tree.get_igraph();
        const igraph_int_t vertex_count = igraph_vcount( graph );

        // the node ID of every vertex, reused for the edges
        std::vector<std::string> node_ids( vertex_count );
        u32 gates_without_coordinates = 0;

        for( igraph_int_t vertex = 0; vertex < vertex_count; vertex++ )
        {
            const auto object = clock_tree.get_ptr_from_vertex( vertex );
            if( object.is_error() )
            {
                return ERR_APPEND( object.get_error(), "couldn't export clock tree to '" + pathname + "'" );
            }
            const auto [ptr, type] = object.get();

            if( type == NET )
            {
                // prefixed, since a numeric net name would otherwise merge with the gate of that ID
                const Net *net = static_cast<const Net *>( ptr );
                node_ids[vertex] = "net_" + std::to_string( net->get_id() );
                dot_file << "  " << node_ids[vertex] << " [label=" << quoted( net->get_name() ) << " shape=circle];\n";
                continue;
            }

            const Gate *gate = static_cast<const Gate *>( ptr );
            node_ids[vertex] = std::to_string( gate->get_id() );
            dot_file << "  " << node_ids[vertex] << " [instance=" << quoted( gate->get_name() )
                     << " type=" << quoted( gate->get_type()->get_name() ) << " shape=" << shape_of( gate );

            // buffers, inverters and delays share the triangle, so the label tells them apart
            if( is_inverter( gate ) )
            {
                dot_file << " label=\"¬ " << node_ids[vertex] << "\"";
            }
            else if( is_delay( gate ) )
            {
                dot_file << " label=\"Δ " << node_ids[vertex] << "\"";
            }

            // coordinates from the gate data can be negative, a location in HAL can't
            std::optional<i32> x;
            std::optional<i32> y;
            if( x_key.has_value() )
            {
                // the Verilog parser stores instance parameters in category "generic"
                x = parse_coordinate( std::get<1>( gate->get_data( "generic", *x_key ) ) );
                y = parse_coordinate( std::get<1>( gate->get_data( "generic", *y_key ) ) );
            }
            else if( gate->has_location() )
            {
                x = gate->get_location_x();
                y = gate->get_location_y();
            }

            if( x.has_value() && y.has_value() )
            {
                dot_file << " x=" << *x << " y=" << *y;
            }
            else
            {
                gates_without_coordinates++;
            }

            dot_file << "];\n";
        }

        // every edge, also those that no root reaches, e.g., in a ring oscillator without an enable
        const igraph_int_t edge_count = igraph_ecount( graph );
        for( igraph_int_t edge = 0; edge < edge_count; edge++ )
        {
            igraph_int_t from;
            igraph_int_t to;
            igraph_edge( graph, edge, &from, &to );  // can't fail for an existing edge ID
            dot_file << "  " << node_ids[from] << " -> " << node_ids[to] << ";\n";
        }

        dot_file << "}\n";
        dot_file.close();
        if( !dot_file )
        {
            return ERR( "couldn't write clock tree to '" + pathname + "'" );
        }

        if( x_key.has_value() && gates_without_coordinates > 0 )
        {
            log_warning( "clock_tree_extractor",
                         "no valid coordinates under keys '{}' and '{}' for {} gate(s)",
                         *x_key,
                         *y_key,
                         gates_without_coordinates );
        }

        return OK( {} );
    }
}  // namespace hal::cte

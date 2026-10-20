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

#include "clock_tree_viewer/clock_tree_layout.h"

#include "clock_tree_extractor/clock_tree.h"
#include "hal_core/netlist/gate.h"

#include <algorithm>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_map>

namespace hal
{
    namespace ctv
    {
        namespace
        {
            // The sizes of the drawing follow from the length of a typical connection, because that decides how large a
            // node can be before it touches its neighbours. The ratios are those of a clock tree with connections of
            // about 25 um that is drawn with a node radius of 1 um, a line width of 0.5 um, a picture margin of 20 um and
            // 2 pixels per um.
            constexpr double REFERENCE_EDGE_LENGTH = 25.0;
            constexpr double NODE_RADIUS_RATIO = 1.0 / REFERENCE_EDGE_LENGTH;
            constexpr double EDGE_WIDTH_RATIO = 0.5 / REFERENCE_EDGE_LENGTH;
            constexpr double PICTURE_MARGIN_RATIO = 20.0 / REFERENCE_EDGE_LENGTH;
            constexpr double EDGE_PICTURE_PIXELS = 2.0 * REFERENCE_EDGE_LENGTH;

            // Without a single connection there is no typical one, a tenth of the extent of the gates takes its place.
            constexpr double EXTENT_RATIO = 0.1;

            struct Bounds
            {
                double min_x;
                double max_x;
                double min_y;
                double max_y;
            };

            std::optional<double> read_coordinate( const Gate *gate, const char *key )
            {
                const std::string value = std::get<1>( gate->get_data( COORDINATE_CATEGORY, key ) );

                const std::size_t first = value.find_first_not_of( " \t\r\n" );
                if( first == std::string::npos )
                {
                    return std::nullopt;
                }
                const std::size_t last = value.find_last_not_of( " \t\r\n" );

                // Read with the classic locale, a coordinate has a decimal point no matter where the program runs.
                std::istringstream stream( value.substr( first, last - first + 1 ) );
                stream.imbue( std::locale::classic() );

                double coordinate = 0.0;
                stream >> coordinate;

                // The whole value must be the number, "12 um" is not a coordinate.
                if( stream.fail() || !stream.eof() || !std::isfinite( coordinate ) )
                {
                    return std::nullopt;
                }

                return coordinate;
            }

            Bounds bounds_of( const Layout &layout )
            {
                Bounds bounds = { .min_x = layout.nodes.front().x,
                                  .max_x = layout.nodes.front().x,
                                  .min_y = layout.nodes.front().y,
                                  .max_y = layout.nodes.front().y };

                for( const Node &node : layout.nodes )
                {
                    bounds.min_x = std::min( bounds.min_x, node.x );
                    bounds.max_x = std::max( bounds.max_x, node.x );
                    bounds.min_y = std::min( bounds.min_y, node.y );
                    bounds.max_y = std::max( bounds.max_y, node.y );
                }

                return bounds;
            }

            double typical_edge_length( const Layout &layout )
            {
                std::vector<double> lengths;
                lengths.reserve( layout.edges.size() );

                for( const Edge &edge : layout.edges )
                {
                    const Node &from = layout.nodes[edge.from];
                    const Node &to = layout.nodes[edge.to];

                    if( const double length = std::hypot( to.x - from.x, to.y - from.y ); length > 0.0 )
                    {
                        lengths.push_back( length );
                    }
                }

                if( !lengths.empty() )
                {
                    const auto middle = lengths.begin() + lengths.size() / 2;
                    std::nth_element( lengths.begin(), middle, lengths.end() );
                    return *middle;
                }

                const Bounds bounds = bounds_of( layout );
                const double extent = std::max( bounds.max_x - bounds.min_x, bounds.max_y - bounds.min_y );

                return extent > 0.0 ? EXTENT_RATIO * extent : 1.0;
            }

            bool is_valid( const std::optional<double> &value, const bool allow_zero )
            {
                return !value.has_value()
                    || ( std::isfinite( *value ) && ( allow_zero ? *value >= 0.0 : *value > 0.0 ) );
            }
        }  // namespace

        Result<Layout> build_layout( const cte::ClockTree &tree )
        {
            std::vector<const Gate *> gates = tree.get_gates();
            if( gates.empty() )
            {
                return ERR( "the clock tree contains no gates" );
            }

            // The clock tree keeps its gates in a hash map, sort them to get the same layout every time.
            std::sort(
                gates.begin(), gates.end(), []( const Gate *a, const Gate *b ) { return a->get_id() < b->get_id(); } );

            Layout layout;
            std::unordered_map<const Gate *, std::size_t> node_of_gate;

            for( const Gate *gate : gates )
            {
                const std::optional<double> x = read_coordinate( gate, COORDINATE_KEY_X );
                const std::optional<double> y = read_coordinate( gate, COORDINATE_KEY_Y );

                if( !x.has_value() || !y.has_value() )
                {
                    layout.gates_without_coordinates.push_back( gate->get_id() );
                    continue;
                }

                node_of_gate[gate] = layout.nodes.size();
                layout.nodes.push_back( { gate->get_id(), *x, *y } );
            }

            if( layout.nodes.empty() )
            {
                return ERR( "none of the " + std::to_string( gates.size() )
                            + " gates of the clock tree has coordinates, expected the gate data ('"
                            + COORDINATE_CATEGORY + "', '" + COORDINATE_KEY_X + "') and ('" + COORDINATE_CATEGORY
                            + "', '" + COORDINATE_KEY_Y + "')" );
            }

            // Nets are not drawn, so the only connections left are those between gates.
            for( const Gate *gate : gates )
            {
                const auto successors = tree.get_neighbors( gate, IGRAPH_OUT );
                if( successors.is_error() )
                {
                    return ERR( "could not read the successors of gate with ID " + std::to_string( gate->get_id() )
                                + ": " + successors.get_error().get() );
                }

                for( const auto &[ptr, type] : successors.get() )
                {
                    if( type != cte::PtrType::GATE )
                    {
                        continue;
                    }

                    const auto from = node_of_gate.find( gate );
                    const auto to = node_of_gate.find( static_cast<const Gate *>( ptr ) );

                    if( from == node_of_gate.end() || to == node_of_gate.end() )
                    {
                        layout.omitted_edges++;
                        continue;
                    }

                    layout.edges.push_back( { from->second, to->second } );
                }
            }

            return OK( std::move( layout ) );
        }

        Result<Style> resolve_style( const Layout &layout, const StyleOverrides &overrides )
        {
            if( layout.nodes.empty() )
            {
                return ERR( "the layout has no gates to size the drawing after" );
            }
            if( !is_valid( overrides.node_radius, false ) )
            {
                return ERR( "node_radius must be a positive number" );
            }
            if( !is_valid( overrides.edge_width, false ) )
            {
                return ERR( "edge_width must be a positive number" );
            }
            if( !is_valid( overrides.picture_margin, true ) )
            {
                return ERR( "picture_margin must not be negative" );
            }
            if( !is_valid( overrides.pixels_per_unit, false ) )
            {
                return ERR( "pixels_per_unit must be a positive number" );
            }

            const double length = typical_edge_length( layout );

            Style style;
            style.node_radius = overrides.node_radius.value_or( NODE_RADIUS_RATIO * length );
            style.edge_width = overrides.edge_width.value_or( EDGE_WIDTH_RATIO * length );
            style.picture_margin = overrides.picture_margin.value_or( PICTURE_MARGIN_RATIO * length );

            if( overrides.pixels_per_unit.has_value() )
            {
                style.pixels_per_unit = *overrides.pixels_per_unit;
            }
            else
            {
                // A typical connection gets the same number of pixels in every picture, unless that would make the
                // picture larger than allowed.
                const Bounds bounds = bounds_of( layout );
                const double side = std::max( bounds.max_x - bounds.min_x, bounds.max_y - bounds.min_y )
                    + 2.0 * ( style.picture_margin + style.node_radius );

                style.pixels_per_unit = std::min( EDGE_PICTURE_PIXELS / length, MAX_PICTURE_SIDE_PIXELS / side );
            }

            return OK( style );
        }
    }  // namespace ctv
}  // namespace hal

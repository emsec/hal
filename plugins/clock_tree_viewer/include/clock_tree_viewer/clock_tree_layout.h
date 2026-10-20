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

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace hal
{
    namespace cte
    {
        class ClockTree;
    }

    /**
     * Everything the clock tree viewer works out from a clock tree before anything is drawn: where the gates are, how
     * they are connected and how large the drawing is. It does not depend on Qt, so it can be tested without a GUI.
     */
    namespace ctv
    {
        /** The gate data that holds the coordinates of a gate, the entries `ClockTree::export_dot` writes to the DOT file. */
        constexpr const char *COORDINATE_CATEGORY = "generic";
        constexpr const char *COORDINATE_KEY_X = "X";
        constexpr const char *COORDINATE_KEY_Y = "Y";

        /** The longest side a picture of the clock tree may have in pixels, which keeps its memory in check. */
        constexpr double MAX_PICTURE_SIDE_PIXELS = 10000.0;

        /** A gate of the clock tree that has coordinates. */
        struct Node
        {
            u32 gate_id;
            double x;
            double y;
        };

        /** A connection from the gate that drives a clock to the gate it clocks, as indices into `Layout::nodes`. */
        struct Edge
        {
            std::size_t from;
            std::size_t to;
        };

        /** The gates of a clock tree at their coordinates, and what had to be left out for lack of them. */
        struct Layout
        {
            /** The gates that have coordinates, sorted by gate ID. */
            std::vector<Node> nodes;

            /** The connections between two gates that both have coordinates. */
            std::vector<Edge> edges;

            /** The IDs of the gates that have no usable coordinates, sorted. They are not drawn. */
            std::vector<u32> gates_without_coordinates;

            /** The number of connections that are not drawn because one of their gates has no coordinates. */
            std::size_t omitted_edges = 0;
        };

        /**
         * The sizes of the drawing the user may choose. Whatever is left unset is derived from the coordinates, see
         * `resolve_style`. All lengths are in the unit of the gate coordinates.
         */
        struct StyleOverrides
        {
            std::optional<double> node_radius;
            std::optional<double> edge_width;
            std::optional<double> picture_margin;
            std::optional<double> pixels_per_unit;
        };

        /** The sizes of the drawing. All lengths are in the unit of the gate coordinates. */
        struct Style
        {
            /** The radius of the dot that represents a gate. */
            double node_radius;

            /** The width of the lines that represent connections. */
            double edge_width;

            /** The empty border around the tree in a picture. */
            double picture_margin;

            /** The resolution of a picture. */
            double pixels_per_unit;
        };

        /**
         * Place the gates of a clock tree at their coordinates.
         *
         * A gate has coordinates if the gate data ('generic', 'X') and ('generic', 'Y') both hold a number. Nets have no
         * coordinates; the only nets in a clock tree are the global inputs at its roots, which are not drawn.
         *
         * @param[in] tree - The clock tree.
         * @returns The layout on success. An error if the clock tree has no gates or none of them has coordinates. Gates
         *          that are only missing some of the coordinates are listed in the layout instead.
         */
        Result<Layout> build_layout( const cte::ClockTree &tree );

        /**
         * Settle the sizes of the drawing.
         *
         * Every size the user did not provide is derived from the length of a typical connection, the median over all
         * connections of the layout: with a typical connection of 25 units, a node has a radius of 1 unit, a line is 0.5
         * units wide, a picture has a margin of 20 units and 2 pixels per unit. For a layout without connections, a tenth
         * of its extent takes the place of the typical connection.
         *
         * @param[in] layout - The layout that is drawn.
         * @param[in] overrides - The sizes provided by the user.
         * @returns The sizes on success, an error if a provided size is not a number or out of range.
         */
        Result<Style> resolve_style( const Layout &layout, const StyleOverrides &overrides );
    }  // namespace ctv
}  // namespace hal

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
#include <string>
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

            /**
             * The resolution of a picture. Unless the user chose it, a picture that would exceed
             * `MAX_PICTURE_SIDE_PIXELS` gets a lower one, see `ClockTreeScene::renderPicture`.
             */
            double pixels_per_unit;

            /** Whether the user chose `pixels_per_unit`, which is then never lowered. */
            bool pixels_per_unit_is_fixed;
        };

        /**
         * Place the gates of a clock tree at their coordinates.
         *
         * The coordinates of a gate are its location in the layout, unless the keys of both coordinates are given: then
         * they are read from the gate parameters of these names, e.g., from Verilog instance parameters, which unlike a
         * location can be negative. Nets have no coordinates; the only nets in a clock tree are the global inputs at its
         * roots, which are not drawn.
         *
         * @param[in] tree - The clock tree.
         * @param[in] x_key - The name of the parameter that holds the x-coordinate, or std::nullopt to use the location.
         * @param[in] y_key - The name of the parameter that holds the y-coordinate, or std::nullopt to use the location.
         * @returns The layout on success. An error if only one key is given, the clock tree has no gates or none of them
         *          has coordinates. Gates that are only missing some of the coordinates are listed in the layout instead.
         */
        Result<Layout> build_layout( const cte::ClockTree &tree,
                                     const std::optional<std::string> &x_key = std::nullopt,
                                     const std::optional<std::string> &y_key = std::nullopt );

        /**
         * Settle the sizes of the drawing.
         *
         * Every size the user did not provide is derived from the length of a typical connection, the median over all
         * connections of the layout: with a typical connection of 25 units, a node has a radius of 1 unit, a line is 0.5
         * units wide, a picture has a margin of 20 units and 2 pixels per unit. For a layout without connections, a tenth
         * of its extent takes the place of the typical connection.
         *
         * The resolution is not limited here: only the scene knows how large the drawing really is, including the width
         * of its lines, so it lowers a derived resolution itself when a picture would be too large.
         *
         * @param[in] layout - The layout that is drawn.
         * @param[in] overrides - The sizes provided by the user.
         * @returns The sizes on success, an error if a provided size is not a number or out of range.
         */
        Result<Style> resolve_style( const Layout &layout, const StyleOverrides &overrides );
    }  // namespace ctv
}  // namespace hal

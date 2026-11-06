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

#include "hal_core/utilities/result.h"

#include <optional>
#include <string>
#include <variant>

namespace hal::cte
{
    class ClockTree;

    /**
     * Write a clock tree to a DOT file.
     *
     * Gates get the coordinates of their location in the layout, unless the keys of both coordinates are given: then
     * they are read from the gate parameters of these names, e.g., from Verilog instance parameters, which unlike a
     * location can be negative.
     *
     * @param[in] clock_tree - The clock tree.
     * @param[in] pathname - The path of the file to write.
     * @param[in] x_key - The name of the parameter that holds the x-coordinate, or std::nullopt to use the location.
     * @param[in] y_key - The name of the parameter that holds the y-coordinate, or std::nullopt to use the location.
     * @returns OK() on success, an error otherwise.
     */
    Result<std::monostate> export_dot( const ClockTree &clock_tree,
                                       const std::string &pathname,
                                       const std::optional<std::string> &x_key = std::nullopt,
                                       const std::optional<std::string> &y_key = std::nullopt );
}  // namespace hal::cte

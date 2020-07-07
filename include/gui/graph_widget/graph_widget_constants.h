//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "QtGlobal"

namespace hal
{
    namespace graph_widget_constants
    {
        static const int font_size = 12;
        static const int grid_size = 14;
        static const int cluster_size = 8;

        static const qreal gate_min_lod = 0.2; // if current lod < than this use lowest level of detail
        static const qreal gate_max_lod = 0.4; // if current lod > than this use highest level of detail

        static const qreal net_min_lod = 0.1; // if current lod < than this use lowest level of detail
        static const qreal net_max_lod = 0.4; // if current lod > than this use highest level of detail

        static const qreal separated_net_min_lod = 0.1; // if current lod < than this use lowest level of detail
        static const qreal separated_net_max_lod = 0.4; // if current lod > than this use highest level of detail

        static const qreal global_net_min_lod = 0.1; // if current lod < than this use lowest level of detail
        static const qreal global_net_max_lod = 0.4; // if current lod > than this use highest level of detail

        static const qreal net_fade_in_lod = 0.1;
        static const qreal net_fade_out_lod = 0.4;

        static const qreal grid_fade_start_lod = 0.4;
        static const qreal grid_fade_end_lod = 1.0;

        static const int drag_swap_sensitivity_distance = 100;

        enum class grid_type
        {
            Lines,
            Dots,
            None
        };
    }
}

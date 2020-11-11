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

#include "gui/graph_widget/layouters/graph_layouter.h"

namespace hal
{
    class StandardGraphLayouter final : public GraphLayouter
    {
    public:
        StandardGraphLayouter(const GraphContext* const context);

        QString name() const override;
        QString mDescription() const override;

        void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, PlacementHint placement) override;
        void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;

    private:
        void addCompact(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
        void addWaitToBeSeated(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
        void addVertical(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, bool left, const Node& preferredOrigin);
    };
}

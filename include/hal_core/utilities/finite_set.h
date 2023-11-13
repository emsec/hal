// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
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

#include <unordered_map>
#include <vector>

namespace hal
{
    template<typename T>
    struct FiniteSet
    {
        FiniteSet(const u32 size);

        bool operator==(const FiniteSet& rhs) const;
        bool operator<(const FiniteSet& rhs) const;
        FiniteSet<T> operator&(const FiniteSet& rhs) const;    // intersect
        FiniteSet<T> operator|(const FiniteSet& rhs) const;    // union
        FiniteSet<T> operator-(const FiniteSet& rhs) const;    // difference
        FiniteSet<T> operator^(const FiniteSet& rhs) const;    // symmetric difference

        bool is_disjoint(const FiniteSet& rhs) const;
        bool is_subset(const FiniteSet& rhs) const;
        bool is_superset(const FiniteSet& rhs) const;

        bool insert(const u32 index);
        bool erase(const u32 index);
        bool contains(const u32 index);

        u32 m_size;
        std::vector<u64> m_content;

    private:
        void initialize();
    };
}    // namespace hal

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

#include <hal_core/utilities/result.h>
#include <unordered_map>
#include <vector>

namespace hal
{
    template<typename T>
    struct FiniteSet
    {
        FiniteSet(const std::vector<T>* const data, const std::unordered_map<T, u32>* const data_to_index) : m_data(data), m_data_to_index(data_to_index)
        {
            m_size    = m_data->size();
            m_content = std::vector<u64>(((m_size - 1) >> 6) + 1, 0);
        }

        bool operator==(const FiniteSet<T>& rhs) const
        {
            return m_content == rhs.m_content;
        }

        bool operator<(const FiniteSet<T>& rhs) const
        {
            return m_content < rhs.m_content;
        }

        FiniteSet<T> operator&(const FiniteSet<T>& rhs) const
        {
            FiniteSet<T> res(m_data, m_data_to_index);
            for (u32 i = 0; i < m_content.size(); i++)
            {
                res.m_content.at(i) = m_content.at(i) & rhs.m_content.at(i);
            }
            return res;
        }

        FiniteSet<T> operator|(const FiniteSet<T>& rhs) const
        {
            FiniteSet<T> res(m_data, m_data_to_index);
            for (u32 i = 0; i < m_content.size(); i++)
            {
                res.m_content.at(i) = m_content.at(i) | rhs.m_content.at(i);
            }
            return res;
        }

        FiniteSet<T> operator-(const FiniteSet<T>& rhs) const
        {
            FiniteSet<T> res(m_data, m_data_to_index);
            for (u32 i = 0; i < m_content.size(); i++)
            {
                res.m_content.at(i) = m_content.at(i) & ~rhs.m_content.at(i);
            }
            return res;
        }

        FiniteSet<T> operator^(const FiniteSet<T>& rhs) const
        {
            FiniteSet<T> res(m_data, m_data_to_index);
            for (u32 i = 0; i < m_content.size(); i++)
            {
                res.m_content.at(i) = m_content.at(i) ^ rhs.m_content.at(i);
            }
            return res;
        }

        bool is_disjoint(const FiniteSet<T>& rhs) const
        {
            for (u32 i = 0; i < m_content.size(); i++)
            {
                if ((m_content.at(i) & rhs.m_content.at(i)) != 0)
                {
                    return false;
                }
            }
            return true;
        }

        bool is_subset(const FiniteSet<T>& rhs) const
        {
            for (u32 i = 0; i < m_content.size(); i++)
            {
                if ((m_content.at(i) & rhs.m_content.at(i)) != m_content.at(i))
                {
                    return false;
                }
            }
            return true;
        }

        bool is_superset(const FiniteSet<T>& rhs) const
        {
            for (u32 i = 0; i < m_content.size(); i++)
            {
                if ((m_content.at(i) & rhs.m_content.at(i)) != rhs.m_content.at(i))
                {
                    return false;
                }
            }
            return true;
        }

        bool insert(const u32 index)
        {
            if (index >= m_size)
            {
                return false;
            }

            m_content.at(index >> 6) ^= (u64)1 << (index & 0x3F);

            return true;
        }

        bool insert(const T& elem)
        {
            const auto it = m_data_to_index->find(elem);
            if (it == m_data_to_index->end())
            {
                return false;
            }

            const u32 index = it->second;
            m_content.at(index >> 6) ^= (u64)1 << (index & 0x3F);
            return true;
        }

        template<typename InputIt>
        bool insert(InputIt begin, InputIt end)
        {
            for (InputIt it = begin; it != end; ++it)
            {
                if (!insert(*it))
                {
                    return false;
                }
            }

            return true;
        }

        bool erase(const u32 index)
        {
            if (index >= m_size)
            {
                return false;
            }

            m_content.at(index >> 6) &= ~((u64)1 << (index & 0x3F));

            return true;
        }

        bool erase(const T& elem)
        {
            const auto it = m_data_to_index->find(elem);
            if (it == m_data_to_index->end())
            {
                return false;
            }

            const u32 index = it->second;
            m_content.at(index >> 6) &= ~((u64)1 << (index & 0x3F));

            return true;
        }

        bool contains(const u32 index)
        {
            if (index >= m_size)
            {
                return false;
            }

            return (m_content.at(index >> 6) & (u64)1 << (index & 0x3F)) != 0;
        }

        bool contains(const T& elem)
        {
            const auto it = m_data_to_index->find(elem);
            if (it == m_data_to_index->end())
            {
                return false;
            }

            const u32 index = it->second;
            return (m_content.at(index >> 6) & (u64)1 << (index & 0x3F)) != 0;
        }

        Result<T> get_at(u32 index) const
        {
            if (index >= m_size)
            {
                return ERR("index " + std::to_string(index) + " is out of range, FiniteSet has size " + std::to_string(m_size));
            }

            return OK(m_data->at(index));
        }

        Result<u32> get_index(const T& elem) const
        {
            const auto it = m_data_to_index->find(elem);
            if (it == m_data_to_index->end())
            {
                return ERR("element cannot be part of FiniteSet because it is not contained in base data");
            }

            return OK(it->second);
        }

        std::vector<T> get_contained() const
        {
            std::vector<T> res;

            for (u32 i = 0; i < m_content.size(); i++)
            {
                for (u32 j = 0; j < 64; j++)
                {
                    if (m_content.at(i) & ((u64)1 << j))
                    {
                        res.push_back(m_data->at((i << 6) + j));
                    }
                }
            }

            return res;
        }

        u32 m_size;
        const std::vector<T>* const m_data;
        const std::unordered_map<T, u32>* const m_data_to_index;
        std::vector<u64> m_content;
    };
}    // namespace hal

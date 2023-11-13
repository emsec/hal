#include "hal_core/utilities/finite_set.h"
#include <iostream>
namespace hal
{
    FiniteSet::FiniteSet(const u32 size) : m_size(size)
    {
        m_content = std::vector<u64>(((m_size - 1) >> 6) + 1, 0);
    }

    bool FiniteSet::operator==(const FiniteSet& rhs) const
    {
        return m_content == rhs.m_content;
    }

    bool FiniteSet::operator<(const FiniteSet& rhs) const
    {
        return m_content < rhs.m_content;
    }

    FiniteSet FiniteSet::operator&(const FiniteSet& rhs) const
    {
        FiniteSet res(m_size);
        for (u32 i = 0; i < m_content.size(); i++)
        {
            res.m_content.at(i) = m_content.at(i) & rhs.m_content.at(i);
        }
        return res;
    }

    FiniteSet FiniteSet::operator|(const FiniteSet& rhs) const
    {
        FiniteSet res(m_size);
        for (u32 i = 0; i < m_content.size(); i++)
        {
            res.m_content.at(i) = m_content.at(i) | rhs.m_content.at(i);
        }
        return res;
    }

    FiniteSet FiniteSet::operator-(const FiniteSet& rhs) const
    {
        FiniteSet res(m_size);
        for (u32 i = 0; i < m_content.size(); i++)
        {
            res.m_content.at(i) = m_content.at(i) & ~rhs.m_content.at(i);
        }
        return res;
    }

    FiniteSet FiniteSet::operator^(const FiniteSet& rhs) const
    {
        FiniteSet res(m_size);
        for (u32 i = 0; i < m_content.size(); i++)
        {
            res.m_content.at(i) = m_content.at(i) ^ rhs.m_content.at(i);
        }
        return res;
    }

    bool FiniteSet::is_disjoint(const FiniteSet& rhs) const
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

    bool FiniteSet::is_subset(const FiniteSet& rhs) const
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

    bool FiniteSet::is_superset(const FiniteSet& rhs) const
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

    bool FiniteSet::insert(const u32 index)
    {
        if (index >= m_size)
        {
            return false;
        }

        m_content.at(index >> 6) ^= (u64)1 << (index & 0x3F);

        return true;
    }

    bool FiniteSet::erase(const u32 index)
    {
        if (index >= m_size)
        {
            return false;
        }

        m_content.at(index >> 6) &= ~((u64)1 << (index & 0x3F));

        return true;
    }

    bool FiniteSet::contains(const u32 index)
    {
        if (index >= m_size)
        {
            return false;
        }

        return (m_content.at(index >> 6) & (u64)1 << (index & 0x3F)) != 0;
    }
}    // namespace hal
#include "hal_core/utilities/finite_set.h"

namespace hal
{
    template<typename T>
    FiniteSet<T>::FiniteSet(const u32 size) : m_size(size)
    {
        m_content = std::vector<u64>((m_size / 64) + 1, 0);
    }

    template<typename T>
    bool FiniteSet<T>::operator==(const FiniteSet& rhs) const
    {
        return m_content == rhs.m_content;
    }

    template<typename T>
    bool FiniteSet<T>::operator<(const FiniteSet& rhs) const
    {
        return m_content < rhs.m_content;
    }

    template<typename T>
    FiniteSet<T> FiniteSet<T>::operator&(const FiniteSet& rhs) const
    {
        FiniteSet<T> res(m_size);
        for (u32 i = 0; i < m_size; i++)
        {
            res.m_content[i] = m_content[i] & rhs.m_content[i];
        }
        return std::move(res);
    }

    template<typename T>
    FiniteSet<T> FiniteSet<T>::operator|(const FiniteSet& rhs) const
    {
        FiniteSet<T> res(m_size);
        for (u32 i = 0; i < m_size; i++)
        {
            res.m_content[i] = m_content[i] | rhs.m_content[i];
        }
        return std::move(res);
    }

    template<typename T>
    FiniteSet<T> FiniteSet<T>::operator-(const FiniteSet& rhs) const
    {
        FiniteSet<T> res(m_size);
        for (u32 i = 0; i < m_size; i++)
        {
            res.m_content[i] = m_content[i] & ~rhs.m_content[i];
        }
        return std::move(res);
    }

    template<typename T>
    FiniteSet<T> FiniteSet<T>::operator^(const FiniteSet& rhs) const
    {
        FiniteSet<T> res(m_size);
        for (u32 i = 0; i < m_size; i++)
        {
            res.m_content[i] = m_content[i] ^ rhs.m_content[i];
        }
        return std::move(res);
    }

    template<typename T>
    bool FiniteSet<T>::is_disjoint(const FiniteSet& rhs) const
    {
        // TODO implement
        return false;
    }

    template<typename T>
    bool FiniteSet<T>::is_subset(const FiniteSet& rhs) const
    {
        // TODO implement
        return false;
    }

    template<typename T>
    bool FiniteSet<T>::is_superset(const FiniteSet& rhs) const
    {
        // TODO implement
        return false;
    }

    template<typename T>
    bool FiniteSet<T>::insert(const u32 index)
    {
        // TODO implement
        return false;
    }

    template<typename T>
    bool FiniteSet<T>::erase(const u32 index)
    {
        // TODO implement
        return false;
    }

    template<typename T>
    bool FiniteSet<T>::contains(const u32 index)
    {
        // TODO implement
        return false;
    }
}    // namespace hal
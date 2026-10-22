#include "bitorder_propagation/bit_order.h"

#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <algorithm>

namespace hal
{
    namespace bitorder_propagation
    {
        BitOrder::BitOrder(Module* module, PinGroup<ModulePin>* pin_group, std::vector<std::pair<Net*, u32>> order)
            : m_module(module), m_pin_group(pin_group), m_order(std::move(order))
        {
            std::sort(m_order.begin(), m_order.end(), [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
        }

        Module* BitOrder::get_module() const
        {
            return m_module;
        }

        PinGroup<ModulePin>* BitOrder::get_pin_group() const
        {
            return m_pin_group;
        }

        const std::vector<std::pair<Net*, u32>>& BitOrder::get_order() const
        {
            return m_order;
        }

        std::optional<u32> BitOrder::get_index(const Net* net) const
        {
            const auto it = std::find_if(m_order.begin(), m_order.end(), [net](const auto& entry) { return entry.first == net; });
            return (it == m_order.end()) ? std::nullopt : std::optional<u32>(it->second);
        }

        Net* BitOrder::get_net_at(u32 index) const
        {
            const auto it = std::find_if(m_order.begin(), m_order.end(), [index](const auto& entry) { return entry.second == index; });
            return (it == m_order.end()) ? nullptr : it->first;
        }

        u32 BitOrder::get_size() const
        {
            return m_order.size();
        }

        bool BitOrder::is_continuous() const
        {
            // m_order is sorted by index, so it suffices that the indices are 0, 1, ... without repetition.
            for (u32 i = 0; i < m_order.size(); i++)
            {
                if (m_order.at(i).second != i)
                {
                    return false;
                }
            }
            return true;
        }

        bool BitOrder::operator==(const BitOrder& other) const
        {
            return (m_module == other.m_module) && (m_pin_group == other.m_pin_group) && (m_order == other.m_order);
        }

        bool BitOrder::operator!=(const BitOrder& other) const
        {
            return !(*this == other);
        }

        namespace
        {
            /// Order by module and pin group ID, so that a result reads the same from one run to the next.
            bool precedes(const BitOrder& lhs, const Module* module, const PinGroup<ModulePin>* pin_group)
            {
                const u32 lhs_module = lhs.get_module()->get_id();
                const u32 rhs_module = module->get_id();
                if (lhs_module != rhs_module)
                {
                    return lhs_module < rhs_module;
                }
                return lhs.get_pin_group()->get_id() < pin_group->get_id();
            }
        }    // namespace

        BitOrderResult::BitOrderResult(std::vector<BitOrder> bit_orders)
        {
            for (auto& bit_order : bit_orders)
            {
                add(std::move(bit_order));
            }
        }

        void BitOrderResult::add(BitOrder bit_order)
        {
            const auto it = std::lower_bound(m_bit_orders.begin(), m_bit_orders.end(), bit_order, [](const BitOrder& lhs, const BitOrder& rhs) {
                return precedes(lhs, rhs.get_module(), rhs.get_pin_group());
            });

            if ((it != m_bit_orders.end()) && (it->get_module() == bit_order.get_module()) && (it->get_pin_group() == bit_order.get_pin_group()))
            {
                *it = std::move(bit_order);
                return;
            }

            m_bit_orders.insert(it, std::move(bit_order));
        }

        const std::vector<BitOrder>& BitOrderResult::get_bit_orders() const
        {
            return m_bit_orders;
        }

        const BitOrder* BitOrderResult::get(const Module* module, const PinGroup<ModulePin>* pin_group) const
        {
            const auto it = std::lower_bound(m_bit_orders.begin(), m_bit_orders.end(), 0, [module, pin_group](const BitOrder& lhs, int) {
                return precedes(lhs, module, pin_group);
            });

            if ((it != m_bit_orders.end()) && (it->get_module() == module) && (it->get_pin_group() == pin_group))
            {
                return &(*it);
            }
            return nullptr;
        }

        bool BitOrderResult::contains(const Module* module, const PinGroup<ModulePin>* pin_group) const
        {
            return get(module, pin_group) != nullptr;
        }

        u32 BitOrderResult::get_size() const
        {
            return m_bit_orders.size();
        }

        bool BitOrderResult::is_empty() const
        {
            return m_bit_orders.empty();
        }

        std::vector<BitOrder>::const_iterator BitOrderResult::begin() const
        {
            return m_bit_orders.begin();
        }

        std::vector<BitOrder>::const_iterator BitOrderResult::end() const
        {
            return m_bit_orders.end();
        }
    }    // namespace bitorder_propagation
}    // namespace hal

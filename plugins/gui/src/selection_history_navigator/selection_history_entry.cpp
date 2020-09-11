#include "gui/selection_history_navigator/selection_history_entry.h"

namespace hal
{
    SelectionHistoryEntry::SelectionHistoryEntry(const QSet<u32> &modIds, const QSet<u32> &gatIds, const QSet<u32> &netIds, int cnt)
        : m_module_ids(modIds), m_gate_ids(gatIds), m_net_ids(netIds), mCount(cnt)
    {;}

    bool SelectionHistoryEntry::isEmpty() const
    {
        return m_module_ids.isEmpty() && m_gate_ids.isEmpty() && m_net_ids.isEmpty();
    }
 }

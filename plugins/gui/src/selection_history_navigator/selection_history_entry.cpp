#include "gui/selection_history_navigator/selection_history_entry.h"

namespace hal
{
    SelectionHistoryEntry::SelectionHistoryEntry(const QSet<u32> &modIds, const QSet<u32> &gatIds, const QSet<u32> &netIds, int cnt)
        : mModuleIds(modIds), mGateIds(gatIds), mNetIds(netIds), mCount(cnt)
    {;}

    bool SelectionHistoryEntry::isEmpty() const
    {
        return mModuleIds.isEmpty() && mGateIds.isEmpty() && mNetIds.isEmpty();
    }
 }

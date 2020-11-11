#include "gui/selection_history_navigator/selection_history_navigator.h"

#include "gui/gui_globals.h"

#include <QList>

namespace hal
{
    SelectionHistoryNavigator::SelectionHistoryNavigator(unsigned int hsize)
        : mCount(0), mHistorySize(hsize)
    {;}

    void SelectionHistoryNavigator::storeCurrentSelection()
    {
        SelectionHistoryEntry she(gSelectionRelay->mSelectedModules,
                                  gSelectionRelay->mSelectedGates,
                                  gSelectionRelay->mSelectedNets,
                                  ++mCount);
        if (she.isEmpty()) return;

        // do not store more than mHistorySize entries
        while (mSelectionContainer.size() >= mHistorySize)
            mSelectionContainer.removeFirst();

        mSelectionContainer.append(she);
    }

    void SelectionHistoryNavigator::emptySelection()
    {
        ++mCount;
    }

    void SelectionHistoryNavigator::restorePreviousEntry()
    {
        if (mSelectionContainer.isEmpty()) return;
        SelectionHistoryEntry she = mSelectionContainer.takeLast();
        if (she.count() == mCount)
        {
            if (mSelectionContainer.isEmpty()) return;
            she = mSelectionContainer.takeLast();
        }
        gSelectionRelay->mSelectedModules = she.mModuleIds;
        gSelectionRelay->mSelectedGates   = she.mGateIds;
        gSelectionRelay->mSelectedNets    = she.mNetIds;
        mCount = she.count();
    }

    bool SelectionHistoryNavigator::hasPreviousEntry() const
    {
        if (mSelectionContainer.isEmpty()) return false;

        // check whether element on top of the history is current selection ...
        if (mSelectionContainer.last().count() != mCount) return true;

        // ... if yes there must be more than one element in history
        return (mSelectionContainer.size() > 1);
    }

}

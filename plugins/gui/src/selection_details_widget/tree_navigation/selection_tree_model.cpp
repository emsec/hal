#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
#include "gui/module_model/module_item.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/netlist/grouping.h"
#include <QTimer>

namespace hal
{
    SelectionTreeModel::SelectionTreeModel(QObject* parent)
        : ModuleModel(parent)
    {
        // Initialise as empty
        fetchSelection(false, 0);
    }

    void SelectionTreeModel::fetchSelection(bool hasEntries, u32 groupingId)
    {
        while(mRootItem->getChildCount() > 0)
            removeChildItem(dynamic_cast<ModuleItem*>(mRootItem->getChild(0)), mRootItem);

        QList<ModuleItem*> newRootList;
        if (!groupingId)
        {
            if (hasEntries)
            {
                for(u32 id : gSelectionRelay->selectedModulesList())
                    addRecursively(gNetlist->get_module_by_id(id));

                for(u32 id : gSelectionRelay->selectedGatesList())
                    newRootList.append(new ModuleItem(id, ModuleItem::TreeItemType::Gate));
                
                for(u32 id : gSelectionRelay->selectedNetsList())
                    newRootList.append(new ModuleItem(id, ModuleItem::TreeItemType::Net));
            }
        }
        else
        {
            Grouping* grouping = gNetlist->get_grouping_by_id(groupingId);
            if (grouping)
            {
                for (u32 id : grouping->get_module_ids())
                    addRecursively(gNetlist->get_module_by_id(id));

                for (u32 id : grouping->get_gate_ids())
                    newRootList.append(new ModuleItem(id, ModuleItem::TreeItemType::Gate));

                for (u32 id : grouping->get_net_ids())
                    newRootList.append(new ModuleItem(id, ModuleItem::TreeItemType::Net));
            }
        }

        setIsModifying(true);
        beginResetModel();
        for(auto item : newRootList)
            mRootItem->appendChild(item);
        setIsModifying(false);
        endResetModel();
    }
}

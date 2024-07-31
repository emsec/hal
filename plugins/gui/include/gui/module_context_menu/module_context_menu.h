#pragma once

#include "gui/module_model/module_item.h"

#include <QMenu>
#include <QSet>

namespace hal {
    class ModuleContextMenu
    {
        public:
            static void addSubmenu(QMenu* contextMenu, const QSet<u32>& modules = QSet<u32>(), const QSet<u32>& gates = QSet<u32>(), const QSet<u32>& nets = QSet<u32>());
            static void addModuleSubmenu(QMenu* contextMenu, u32 id);
            static void addGateSubmenu(QMenu* contextMenu, u32 id);
            static void addNetSubmenu(QMenu* contextMenu, u32 id);
            static void addMultipleElementsSubmenu(QMenu* contextMenu, const QSet<u32>& modules = QSet<u32>(), const QSet<u32>& gates = QSet<u32>(), const QSet<u32>& nets = QSet<u32>());

            /**
             * Changes the name of a specific netlist element by asking the user for a new name in a dialogue.
             *
             * @param type - The type of the element to rename
             * @param id - The id of the element to rename
             */
            static void changeElementNameDialog(ModuleItem::TreeItemType type, u32 id);
            /**
             * Changes the type of a specific module by asking the user for a new type in a dialogue.
             *
             * @param id - The id of the moudle to change to type of
             */
            static void changeModuleTypeDialog(const u32 id);
    };
}

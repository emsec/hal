#pragma once

#include "hal_core/defines.h"

#include <QMenu>
#include <QSet>

namespace hal {
    class ModuleContextMenu
    {
        public:
            /**
             * Adds general context menu actions for a specified module.
             * 
             * @param contextMenu - The QMenu to which the actions are added to.
             * @param id - The id of the module on which the actions will operate.
             */
            static void addModuleSubmenu(QMenu* contextMenu, u32 id);
            /**
             * Adds general context menu actions for a specified gate.
             * 
             * @param contextMenu - The QMenu to which the actions are added to.
             * @param id - The id of the gate on which the actions will operate.
             */
            static void addGateSubmenu(QMenu* contextMenu, u32 id);
            /**
             * Adds general context menu actions for a specified net.
             * 
             * @param contextMenu - The QMenu to which the actions are added to.
             * @param id - The id of the net on which the actions will operate.
             */
            static void addNetSubmenu(QMenu* contextMenu, u32 id);
            /**
             * Adds general context menu actions for multiple specified netlist elements.
             * 
             * @param contextMenu - The QMenu to which the actions are added to.
             * @param modules - QSet with ids of modules on which the actions will operate.
             * @param gates - QSet with ids of gates on which the actions will operate.
             * @param nets - QSet with ids of nets on which the actions will operate.
             */
            static void addMultipleElementsSubmenu(QMenu* contextMenu, const QSet<u32>& modules = QSet<u32>(), const QSet<u32>& gates = QSet<u32>(), const QSet<u32>& nets = QSet<u32>());
    };
}

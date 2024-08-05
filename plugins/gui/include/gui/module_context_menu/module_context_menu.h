#pragma once

#include "hal_core/defines.h"

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
    };
}

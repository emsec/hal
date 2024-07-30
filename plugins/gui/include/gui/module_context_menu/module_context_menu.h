#pragma once

#include "gui/module_model/module_item.h"

#include <QMenu>

namespace hal {
    class ModuleContextMenu
    {
        public:
            static void addSubmenu(QMenu* contextMenu, ModuleItem::TreeItemType type, u32 id);
            static void addModuleSubmenu(QMenu* contextMenu, u32 id);
            static void addGateSubmenu(QMenu* contextMenu, u32 id);
            static void addNetSubmenu(QMenu* contextMenu, u32 id);
    };
}

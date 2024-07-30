#include "gui/module_context_menu/module_context_menu.h"

#include "hal_core/defines.h"
#include "gui/gui_globals.h"

#include <QApplication>
#include <QClipboard>

namespace hal {
    void ModuleContextMenu::addSubmenu(QMenu* contextMenu, ModuleItem::TreeItemType type, u32 id)
    {
        switch(type)
        {
            case ModuleItem::TreeItemType::Module: addModuleSubmenu(contextMenu, id); break;
            case ModuleItem::TreeItemType::Gate: addGateSubmenu(contextMenu, id); break;
            case ModuleItem::TreeItemType::Net: addNetSubmenu(contextMenu, id); break;
        }
    }

    void ModuleContextMenu::addModuleSubmenu(QMenu* contextMenu, u32 id)
    {
        QMenu* menu = contextMenu->addMenu("Module");
        menu->addAction("Name to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name()));
           }
        );
        menu->addAction("ID to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::number(id));
           }
        );
        menu->addAction("Type to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_module_by_id(id)->get_type()));
           }
        );
    }

    void ModuleContextMenu::addGateSubmenu(QMenu* contextMenu, u32 id)
    {
        QMenu* menu = contextMenu->addMenu("Gate");
        menu->addAction("Name to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_gate_by_id(id)->get_name()));
           }
        );
        menu->addAction("ID to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::number(id));
           }
        );
        menu->addAction("Type to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_gate_by_id(id)->get_type()->get_name()));
           }
        );
    }

    void ModuleContextMenu::addNetSubmenu(QMenu* contextMenu, u32 id)
    {
        QMenu* menu = contextMenu->addMenu("Gate");
        menu->addAction("Name to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_net_by_id(id)->get_name()));
           }
        );
        menu->addAction("ID to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::number(id));
           }
        );
    }
}
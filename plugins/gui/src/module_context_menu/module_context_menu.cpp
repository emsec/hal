#include "gui/module_context_menu/module_context_menu.h"

#include "hal_core/defines.h"
#include "gui/gui_globals.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_type.h"
#include "gui/grouping_dialog/grouping_dialog.h"
#include "gui/content_manager/content_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>

namespace hal {
    /**void ModuleContextMenu::addSubmenu(QMenu* contextMenu, const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, ) // extra id and type for showing all selected menu and clicked menu
    {
        if(modules.size() == 1 && gates.size() <= 0 && nets.size() <= 0) 
            addModuleSubmenu(contextMenu, *(modules.begin()));
        else if(modules.size() <= 0 && gates.size() == 1 && nets.size() <= 0) 
            addGateSubmenu(contextMenu, *(gates.begin()));
        else if(modules.size() <= 0 && gates.size() <= 0 && nets.size() == 1) 
            addNetSubmenu(contextMenu, *(nets.begin()));
        else if(modules.size() >=1 || gates.size() >= 1 || nets.size() >= 1) 
            addMultipleElementsSubmenu(contextMenu, modules, gates, nets);
    }**/

    void ModuleContextMenu::addSubmenu(QMenu* contextMenu, const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        if(modules.size() == 1 && gates.size() <= 0 && nets.size() <= 0) 
            addModuleSubmenu(contextMenu, *(modules.begin()));
        else if(modules.size() <= 0 && gates.size() == 1 && nets.size() <= 0) 
            addGateSubmenu(contextMenu, *(gates.begin()));
        else if(modules.size() <= 0 && gates.size() <= 0 && nets.size() == 1) 
            addNetSubmenu(contextMenu, *(nets.begin()));
        else if(modules.size() >=1 || gates.size() >= 1 || nets.size() >= 1) 
            addMultipleElementsSubmenu(contextMenu, modules, gates, nets);
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
        menu->addAction(QIcon(":/icons/python"), 
            "Extract module as python code (copy to clipboard)", 
            [id]()
            {
                QApplication::clipboard()->setText("netlist.get_module_by_id(" + QString::number(id) + ")");
            }
        );
        menu->addAction("Change name",
           [id]()
           {
               ModuleContextMenu::changeElementNameDialog(ModuleItem::TreeItemType::Module, id);
           }
        );
        menu->addAction("Assign to grouping",
           [id]()
           {
                gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({id});
           }
        );
        menu->addAction("Remove from grouping",
           [id]()
           {
                gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({id});
           }
        );
        menu->addAction("Focus in Graph View",
           [id]()
           {
                gContentManager->getGraphTabWidget()->handleModuleFocus(id);
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
        menu->addAction(QIcon(":/icons/python"), 
            "Extract gate as python code (copy to clipboard)", 
            [id]()
            {
                QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(id) + ")");
            }
        );
        menu->addAction("Change name",
           [id]()
           {
               ModuleContextMenu::changeElementNameDialog(ModuleItem::TreeItemType::Gate, id);
           }
        );
        menu->addAction("Assign to grouping",
           [id]()
           {
                gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({},{id});
           }
        );
        menu->addAction("Remove from grouping",
           [id]()
           {
                gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({},{id});
           }
        );
        menu->addAction("Focus in Graph View",
           [id]()
           {
                gContentManager->getGraphTabWidget()->handleGateFocus(id);
           }
        );
    }

    void ModuleContextMenu::addNetSubmenu(QMenu* contextMenu, u32 id)
    {
        QMenu* menu = contextMenu->addMenu("Net");
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
        menu->addAction(QIcon(":/icons/python"), 
            "Extract net as python code (copy to clipboard)", 
            [id]()
            {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + QString::number(id) + ")");
            }
        );
        menu->addAction("Change name",
           [id]()
           {
               ModuleContextMenu::changeElementNameDialog(ModuleItem::TreeItemType::Net, id);
           }
        );
        menu->addAction("Assign to grouping",
           [id]()
           {
                gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({},{},{id});
           }
        );
        menu->addAction("Remove from grouping",
           [id]()
           {
                gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({},{},{id});
           }
        );
        menu->addAction("Focus in Graph View",
           [id]()
           {
                gContentManager->getGraphTabWidget()->handleNetFocus(id);
           }
        );
    }

    void ModuleContextMenu::addMultipleElementsSubmenu(QMenu* contextMenu, const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        QMenu* menu = contextMenu->addMenu("Modules, Gates, Nets");
        menu->addAction("Assign to grouping",
           [modules, gates, nets]()
           {
                gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog(modules, gates, nets);
           }
        );
        menu->addAction("Remove from grouping",
           [modules, gates, nets]()
           {
                gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping(modules, gates, nets);
           }
        );
    }

    void ModuleContextMenu::changeElementNameDialog(ModuleItem::TreeItemType type, u32 id)
    {
        QString oldName;
        QString prompt;
        UserActionObject uao;

        if (type == ModuleItem::TreeItemType::Module)
        {
            Module* m = gNetlist->get_module_by_id(id);
            assert(m);
            oldName   = QString::fromStdString(m->get_name());
            prompt    = "Change module name";
            uao = UserActionObject(id, UserActionObjectType::Module);
        }
        else if (type == ModuleItem::TreeItemType::Gate)
        {
            Gate* g   = gNetlist->get_gate_by_id(id);
            assert(g);
            oldName   = QString::fromStdString(g->get_name());
            prompt    = "Change gate name";
            uao = UserActionObject(id, UserActionObjectType::Gate);
        }
        else if (type == ModuleItem::TreeItemType::Net)
        {
            Net* n    = gNetlist->get_net_by_id(id);
            assert(n);
            oldName   = QString::fromStdString(n->get_name());
            prompt    = "Change net name";
            uao = UserActionObject(id, UserActionObjectType::Net);
        }
        else return;

        bool confirm;
        QString newName =
                QInputDialog::getText(nullptr, prompt, "New name:", QLineEdit::Normal,
                                      oldName, &confirm);
        if (confirm)
        {
            ActionRenameObject* act = new ActionRenameObject(newName);
            act->setObject(uao);
            act->exec();
        }
    }

    void ModuleContextMenu::changeModuleTypeDialog(const u32 id)
    {
        // NOT THREADSAFE

        Module* m = gNetlist->get_module_by_id(id);
        assert(m);

        bool ok;
        QString type = QInputDialog::getText(nullptr, "Change Module Type", "New Type", QLineEdit::Normal, QString::fromStdString(m->get_type()), &ok);

        if (ok)
        {
            ActionSetObjectType* act = new ActionSetObjectType(type);
            act->setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
            act->exec();
        }
    }
}
#include "gui/module_context_menu/module_context_menu.h"

#include "hal_core/defines.h"
#include "gui/gui_globals.h"
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
        contextMenu->addSeparator();
        contextMenu->addAction("This module (" + QString::number(id)+"):")->setDisabled(true);
        contextMenu->addAction("  Module name to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name()));}
        );
        contextMenu->addAction("  Module ID to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::number(id));}
        );
        contextMenu->addAction("  Module type to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_module_by_id(id)->get_type()));}
        );
        contextMenu->addAction(QIcon(":/icons/python"), 
            "  Extract module as python code (copy to clipboard)", 
            [id]()
            {QApplication::clipboard()->setText("netlist.get_module_by_id(" + QString::number(id) + ")");}
        );
        contextMenu->addAction("  Change module name",
           [id]()
           {gNetlistRelay->changeElementNameDialog(ModuleItem::TreeItemType::Module, id);}
        );
        contextMenu->addAction("  Change module type",
           [id]()
           {gNetlistRelay->changeModuleTypeDialog(id);}
        );
        contextMenu->addAction("  Change module color",
           [id]()
           {gNetlistRelay->changeModuleColorDialog(id);}
        );
        contextMenu->addAction("  Add child module", 
            [id]()
            {gNetlistRelay->addChildModuleDialog(id);}
        );
        contextMenu->addAction("  Add selected gates to module", 
            [id]()
            {gNetlistRelay->addSelectionToModule(id);}
        );
        contextMenu->addAction("  Set module as selection",
            [id](){
                gSelectionRelay->setSelectedModules({id});
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        contextMenu->addAction("  Add module to selection",
            [id](){
                gSelectionRelay->addModule(id);
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        contextMenu->addAction("  Assign module to grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({id});}
        );
        contextMenu->addAction("  Remove module from grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({id});}
        );
        contextMenu->addAction("  Focus module in Graph View",
           [id]()
           {gContentManager->getGraphTabWidget()->handleModuleFocus(id);}
        );
        contextMenu->addAction("  Isolate module in new view", 
            [id]()
            {gGraphContextManager->openModuleInView(id, false);}
        );
        QAction* delAction = contextMenu->addAction("  Delete module", 
            [id]()
            {gNetlistRelay->deleteModule(id);}
        );
        if(gNetlist->get_module_by_id(id) == gNetlist->get_top_module())
            delAction->setEnabled(false);
    }

    void ModuleContextMenu::addGateSubmenu(QMenu* contextMenu, u32 id)
    {
        QAction *act;

        contextMenu->addSeparator();
        contextMenu->addAction("This gate (" + QString::number(id)+"):")->setDisabled(true);
        contextMenu->addAction("  Gate name to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_gate_by_id(id)->get_name()));}
        );
        contextMenu->addAction("  Gate ID to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::number(id));}
        );
        contextMenu->addAction("  Gate type to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_gate_by_id(id)->get_type()->get_name()));}
        );
        contextMenu->addAction(QIcon(":/icons/python"), 
            "  Extract gate as python code (copy to clipboard)", 
            [id]()
            {QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(id) + ")");}
        );
        contextMenu->addAction("  Change gate name",
           [id]()
           {gNetlistRelay->changeElementNameDialog(ModuleItem::TreeItemType::Gate, id);}
        );
        contextMenu->addAction("  Set gate as selection",
            [id](){
                gSelectionRelay->setSelectedGates({id});
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        act = contextMenu->addAction("  Add gate to selection",
            [id](){
                gSelectionRelay->addGate(id);
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        if(gSelectionRelay->selectedGates().contains(id))
            act->setEnabled(false);
        contextMenu->addAction("  Assign gate to grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({},{id});}
        );
        contextMenu->addAction("  Remove gate from grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({},{id});}
        );
        contextMenu->addAction("  Focus gate in Graph View",
           [id]()
           {gContentManager->getGraphTabWidget()->handleGateFocus(id);}
        );
        contextMenu->addAction("  Isolate gate in new view", 
            [id]()
            {gGraphContextManager->openGateInView(id);}
        );
    }

    void ModuleContextMenu::addNetSubmenu(QMenu* contextMenu, u32 id)
    {
        contextMenu->addSeparator();
        contextMenu->addAction("This net (" + QString::number(id)+"):")->setDisabled(true);
        contextMenu->addAction("  Net name to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::fromStdString(gNetlist->get_net_by_id(id)->get_name()));}
        );
        contextMenu->addAction("  Net ID to clipboard",
           [id]()
           {QApplication::clipboard()->setText(QString::number(id));}
        );
        contextMenu->addAction(QIcon(":/icons/python"), 
            "  Extract net as python code (copy to clipboard)", 
            [id]()
            {QApplication::clipboard()->setText("netlist.get_net_by_id(" + QString::number(id) + ")");}
        );
        contextMenu->addAction("  Change net name",
           [id]()
           {gNetlistRelay->changeElementNameDialog(ModuleItem::TreeItemType::Net, id);}
        );
        contextMenu->addAction("  Set net as selection",
            [id]()
            {
                gSelectionRelay->setSelectedNets({id});
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        contextMenu->addAction("  Add net to selection",
            [id]()
            {
                gSelectionRelay->addNet(id);
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
         );
        contextMenu->addAction("  Assign net to grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({},{},{id});}
        );
        contextMenu->addAction("  Remove net from grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({},{},{id});}
        );
        contextMenu->addAction("  Focus net in Graph View",
           [id]()
           {gContentManager->getGraphTabWidget()->handleNetFocus(id);}
        );
        contextMenu->addAction("  Isolate net endpoints in new view", 
            [id]()
            {gGraphContextManager->openNetEndpointsInView(id);}
        );
    }

    void ModuleContextMenu::addMultipleElementsSubmenu(QMenu* contextMenu, const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        contextMenu->addSeparator();
        contextMenu->addAction("Entire selection:")->setDisabled(true);
        contextMenu->addAction("  Assign all to grouping",
           [modules, gates, nets]()
           {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog(modules, gates, nets);}
        );
        contextMenu->addAction("  Remove all from grouping",
           [modules, gates, nets]()
           {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping(modules, gates, nets);}
        );
    }
}
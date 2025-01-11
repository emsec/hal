#include "gui/module_context_menu/module_context_menu.h"

#include "hal_core/defines.h"
#include "gui/gui_globals.h"
#include "gui/grouping_dialog/grouping_dialog.h"
#include "gui/content_manager/content_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/python/py_code_provider.h"
#include "gui/user_action/action_set_selection_focus.h"

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>

namespace hal {
    void ModuleContextMenu::addModuleSubmenu(QMenu* contextMenu, u32 id)
    {
        QAction* act;
        Module* module = gNetlist->get_module_by_id(id);

        contextMenu->addSeparator();
        contextMenu->addAction("This module (ID:" + QString::number(id)+"):")->setDisabled(true);
        QMenu *sm = contextMenu->addMenu("  To clipboard …");
        sm->addAction("Module name",
           [id, module]()
           {QApplication::clipboard()->setText(QString::fromStdString(module->get_name()));}
        );
        sm->addAction("Module ID",
           [id]()
           {QApplication::clipboard()->setText(QString::number(id));}
        );
        sm->addAction("Module type",
           [id, module]()
           {QApplication::clipboard()->setText(QString::fromStdString(module->get_type()));}
        );
        sm->addAction(QIcon(":/icons/python"),
            "  Module as python code", 
            [id]()
            {QApplication::clipboard()->setText(PyCodeProvider::pyCodeModule(id));}
        );
        sm = contextMenu->addMenu("  Change …");
        sm->addAction("Module name",
           [id]()
           {gNetlistRelay->changeElementNameDialog(ModuleItem::TreeItemType::Module, id);}
        );
        sm->addAction("Module type",
           [id]()
           {gNetlistRelay->changeModuleTypeDialog(id);}
        );
        sm->addAction("Module color",
           [id]()
           {gNetlistRelay->changeModuleColorDialog(id);}
        );
        contextMenu->addAction("  Add child module", 
            [id]()
            {gNetlistRelay->addChildModuleDialog(id);}
        );
        sm = contextMenu->addMenu("  To selection …");
        sm->addAction("Set module as selection",
            [id](){
                ActionSetSelectionFocus *assf = new ActionSetSelectionFocus();
                assf->setObject(UserActionObject(id, UserActionObjectType::Module));
                assf->exec();
            }
        );
        act = sm->addAction("Add module to selection",
            [id](){
                gSelectionRelay->addModule(id);
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        if(gSelectionRelay->selectedModules().contains(id))
            act->setEnabled(false);

        if (gSelectionRelay->numberSelectedNodes() == 1)
            contextMenu->addAction("  Move to module …",
                [id](){
                    gNetlistRelay->addToModuleDialog(Node(id,Node::Module));
                }
            );
        sm = contextMenu->addMenu("  To grouping …");
        QString actionText = "  Assign module to grouping";
        if(module->get_grouping() != nullptr)
            actionText = "  Reassign module to grouping";
        sm->addAction(actionText,
            [id]()
            {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({id});}
        );

        act = sm->addAction("  Remove module from grouping",
            [id]()
            {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({id});}
        );
        if(module->get_grouping() == nullptr)
            act->setEnabled(false);

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
        if(module == gNetlist->get_top_module())
            delAction->setEnabled(false);
    }

    void ModuleContextMenu::addGateSubmenu(QMenu* contextMenu, u32 id)
    {
        QAction *act;
        Gate* gate = gNetlist->get_gate_by_id(id);

        contextMenu->addSeparator();
        contextMenu->addAction("This gate (ID:" + QString::number(id)+"):")->setDisabled(true);
        QMenu *sm = contextMenu->addMenu("  To clipboard …");
        sm->addAction("Gate name",
           [id, gate]()
           {QApplication::clipboard()->setText(QString::fromStdString(gate->get_name()));}
        );
        sm->addAction("Gate ID",
           [id]()
           {QApplication::clipboard()->setText(QString::number(id));}
        );
        sm->addAction("Gate type",
           [id, gate]()
           {QApplication::clipboard()->setText(QString::fromStdString(gate->get_type()->get_name()));}
        );
        sm->addAction(QIcon(":/icons/python"), 
            "  Gate as python code", 
            [id]()
            {QApplication::clipboard()->setText(PyCodeProvider::pyCodeGate(id));}
        );
        contextMenu->addAction("  Change gate name",
           [id]()
           {gNetlistRelay->changeElementNameDialog(ModuleItem::TreeItemType::Gate, id);}
        );
        sm = contextMenu->addMenu("  To selection …");
        sm->addAction("  Set gate as selection",
            [id](){
                ActionSetSelectionFocus *assf = new ActionSetSelectionFocus();
                assf->setObject(UserActionObject(id, UserActionObjectType::Gate));
                assf->exec();
            }
        );
        act = sm->addAction("  Add gate to selection",
            [id](){
                gSelectionRelay->addGate(id);
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        if(gSelectionRelay->selectedGates().contains(id))
            act->setEnabled(false);

        if (gSelectionRelay->numberSelectedNodes() == 1)
            contextMenu->addAction("  Move to module …",
                [id](){
                    gNetlistRelay->addToModuleDialog(Node(id,Node::Gate));
                }
            );
        sm = contextMenu->addMenu("  To grouping …");
        QString actionText = "  Assign gate to grouping";
        if(gate->get_grouping() != nullptr)
            actionText = "  Reassign gate to grouping";
        sm->addAction(actionText,
            [id]()
            {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({},{id});}
        );

        act = sm->addAction("  Remove gate from grouping",
            [id]()
            {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({},{id});}
        );
        if(gate->get_grouping() == nullptr)
            act->setEnabled(false);
        
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
        QAction* act;
        Net* net = gNetlist->get_net_by_id(id);

        contextMenu->addSeparator();
        contextMenu->addAction("This net (ID:" + QString::number(id)+"):")->setDisabled(true);
        QMenu *sm = contextMenu->addMenu("  To clipboard …");
        sm->addAction("Net name",
           [id, net]()
           {QApplication::clipboard()->setText(QString::fromStdString(net->get_name()));}
        );
        sm->addAction("Net ID",
           [id]()
           {QApplication::clipboard()->setText(QString::number(id));}
        );
        sm->addAction(QIcon(":/icons/python"), 
            "  Net as python code", 
            [id]()
            {QApplication::clipboard()->setText(PyCodeProvider::pyCodeNet(id));}
        );
        contextMenu->addAction("  Change net name",
           [id]()
           {gNetlistRelay->changeElementNameDialog(ModuleItem::TreeItemType::Net, id);}
        );
        sm = contextMenu->addMenu("  To selection …");
        sm->addAction("Set net as selection",
            [id]()
            {
                ActionSetSelectionFocus *assf = new ActionSetSelectionFocus();
                assf->setObject(UserActionObject(id, UserActionObjectType::Net));
                assf->exec();
            }
        );
        act = sm->addAction("Add net to selection",
            [id]()
            {
                gSelectionRelay->addNet(id);
                gSelectionRelay->relaySelectionChanged(nullptr);
            }
        );
        if(gSelectionRelay->selectedNets().contains(id))
            act->setEnabled(false);

        sm = contextMenu->addMenu("  To grouping …");
        QString actionText = "  Assign net to grouping";
        if(net->get_grouping() != nullptr)
            actionText = "  Reassign net to grouping";
        sm->addAction(actionText,
           [id]()
           {gContentManager->getGroupingManagerWidget()->assignElementsToGroupingDialog({},{},{id});}
        );

        act = sm->addAction("  Remove net from grouping",
           [id]()
           {gContentManager->getGroupingManagerWidget()->removeElementsFromGrouping({},{},{id});}
        );
        if(net->get_grouping() == nullptr)
            act->setEnabled(false);

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
        if (gSelectionRelay->numberSelectedNodes() > 1)
            contextMenu->addAction("  Move to module …",
                [](){
                    gNetlistRelay->addToModuleDialog();
                }
            );

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

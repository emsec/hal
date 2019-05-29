#include "graph_manager/graph_graphics_widget.h"
#include "graph_layouter/input_dialog.h"
#include "gui_globals.h"
#include "gui_utility.h"
#include "netlist/module.h"
#include <QColorDialog>
#include <QDebug>
#include <QOpenGLWidget>
#include <QSignalMapper>

graph_graphics_widget::graph_graphics_widget(QGraphicsView* view) : m_view(view), m_background(&m_background_scene, this), m_layout(this)
{
    m_background.setFrameShape(QFrame::NoFrame);
    m_view->setFrameShape(QFrame::NoFrame);

    m_layout.setContentsMargins(0, 0, 0, 0);
    m_background_layout.setContentsMargins(0, 0, 0, 0);
    m_background_layout.addWidget(&m_background);
    m_layout.addLayout(&m_background_layout, 0, 0, 1, 1);
    m_layout.addWidget(m_view, 0, 0, 1, 1);

    connect(m_view, SIGNAL(redrawn()), this, SLOT(resize_background()));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
}

void graph_graphics_widget::resize_background()
{
    int right_margin  = 0;
    int bottom_margin = 0;
    if (m_view->verticalScrollBar()->isVisible())
    {
        right_margin = 14;    //magic number until i find a way to get the correct size of the frame generically
    }
    if (m_view->horizontalScrollBar()->isVisible())
    {
        bottom_margin = 14;    //magic number until i find a way to get the correct size of the frame generically
    }
    m_background_layout.setContentsMargins(0, 0, right_margin, bottom_margin);
}

void graph_graphics_widget::ShowContextMenu(const QPoint& pos)
{
    QMenu contextMenu2("Context Menu", this);

    if (m_view->itemAt(pos))
    {
        QAction edit_name_action("Edit name");
        QGraphicsItem* item      = m_view->itemAt(pos);
        gui_graph_gate* gui_gate = dynamic_cast<gui_graph_gate*>(item);
        //makes the whole life a lot easier
        if (gui_gate)
        {
            gui_gate->setSelected(true);
            current_context_gate = gui_gate;
            connect(&edit_name_action, &QAction::triggered, this, &graph_graphics_widget::edit_gate_data);
        }
        else
        {
            gui_graph_net* gui_net = dynamic_cast<gui_graph_net*>(item);
            if (gui_net)
            {
                gui_net->setSelected(true);
                current_context_net = gui_net;
                connect(&edit_name_action, &QAction::triggered, this, &graph_graphics_widget::edit_net_data);
            }
        }

        currently_selected_gates = dynamic_cast<graph_layouter_view*>(m_view)->get_selected_gates();
        currently_selected_nets  = dynamic_cast<graph_layouter_view*>(m_view)->get_selected_nets();

        QAction gate_action_new_module("N");
        QMenu* gate_menu_existing_module = nullptr;
        QAction net_action_new_module("N");
        QMenu* net_menu_existing_submoudle = nullptr;
        QAction all_action_new_module("N");
        QMenu* all_menu_existing_module = nullptr;
        QMenu* gate_menu_remove_module  = nullptr;
        QMenu* net_menu_remove_module   = nullptr;
        QMenu* all_menu_remove_module   = nullptr;
        bool exist_modules              = !g_netlist->get_modules().empty();

        //gate actions, check if it is needed, and if yes, if "gates" or gate
        if (currently_selected_gates.size() > 1)
        {
            gate_action_new_module.setText("Add gates to new module...");
            if (exist_modules)
                gate_menu_existing_module = new QMenu("Add gates to existing module");    //contextMenu2.addMenu("Add Gates to existing module...");
            gate_menu_remove_module = new QMenu("Remove gates from module");
        }
        if (currently_selected_gates.size() == 1)
        {
            gate_action_new_module.setText("Add Gate to new module...");
            if (exist_modules)
                gate_menu_existing_module = new QMenu("Add gate to existing module");    //contextMenu2.addMenu("Add Gate to exisiting module...");
            gate_menu_remove_module = new QMenu("Remove gate from module");
        }
        //gate connections
        if (gate_action_new_module.text() != "N")
        {
            connect(&gate_action_new_module, &QAction::triggered, this, &graph_graphics_widget::handle_add_gates_to_new_module_triggered);
            contextMenu2.addAction(&gate_action_new_module);
        }
        if (gate_menu_existing_module)
        {
            for (const auto& submod : g_netlist->get_modules())
            {
                QAction* tempAction = new QAction(QString::fromStdString(submod->get_name()) + " [ID:  " + QString::number((int)submod->get_id()) + "]", this);
                tempAction->setData(QVariant(submod->get_id()));
                connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_add_gates_to_existing_module);
                gate_menu_existing_module->addAction(tempAction);
            }
            contextMenu2.addMenu(gate_menu_existing_module);
        }

        //check the nets
        if (currently_selected_nets.size() > 1)
        {
            net_action_new_module.setText("Add nets to new module...");
            if (exist_modules)
                net_menu_existing_submoudle = new QMenu("Add nets to existing module");
            net_menu_remove_module = new QMenu("Remove nets from module");
        }
        if (currently_selected_nets.size() == 1)
        {
            net_action_new_module.setText("Add net to new module...");
            if (exist_modules)
                net_menu_existing_submoudle = new QMenu("Add net to exisiting module");
            net_menu_remove_module = new QMenu("Remove net from module");
        }
        //net connections
        if (net_action_new_module.text() != "N")
        {
            connect(&net_action_new_module, &QAction::triggered, this, &graph_graphics_widget::handle_add_nets_to_new_module_triggered);
            contextMenu2.addAction(&net_action_new_module);
        }
        if (net_menu_existing_submoudle)
        {
            for (const auto& submod : g_netlist->get_modules())
            {
                QAction* tempAction = new QAction(QString::fromStdString(submod->get_name()) + " [ID:  " + QString::number((int)submod->get_id()) + "]", this);
                tempAction->setData(QVariant(submod->get_id()));
                connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_add_nets_to_existing_module_triggered);
                net_menu_existing_submoudle->addAction(tempAction);
            }
            contextMenu2.addMenu(net_menu_existing_submoudle);
        }

        //Both nets and gates
        if (currently_selected_gates.size() >= 1 && currently_selected_nets.size() >= 1)
        {
            all_action_new_module.setText("Add all to new module...");
            if (exist_modules)
                all_menu_existing_module = new QMenu("Add all to existing module");
            all_menu_remove_module = new QMenu("Remove all from module");
        }
        if (all_action_new_module.text() != "N")
        {
            connect(&all_action_new_module, &QAction::triggered, this, &graph_graphics_widget::handle_add_gates_and_nets_to_new_module_triggered);
            contextMenu2.addAction(&all_action_new_module);
        }
        if (all_menu_existing_module)
        {
            for (const auto& submod : g_netlist->get_modules())
            {
                QAction* tempAction = new QAction(QString::fromStdString(submod->get_name()) + " [ID:  " + QString::number((int)submod->get_id()) + "]", this);
                tempAction->setData(QVariant(submod->get_id()));
                connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_add_gates_and_nets_to_existing_module_triggered);
                all_menu_existing_module->addAction(tempAction);
            }
            contextMenu2.addMenu(all_menu_existing_module);
        }
        if (gate_menu_remove_module)
        {
            QList<std::shared_ptr<module>> subs_found;
            for (const auto& sub : g_netlist->get_modules())
            {
                for (const auto& gate : currently_selected_gates)
                {
                    if (sub->contains_gate(gate->get_ref_gate()) && !subs_found.contains(sub))
                    {
                        QAction* tempAction = new QAction(QString::fromStdString(sub->get_name()) + " [ID:  " + QString::number((int)sub->get_id()) + "]", this);
                        tempAction->setData(QVariant(sub->get_id()));
                        connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_remove_gates_from_module_triggered);
                        gate_menu_remove_module->addAction(tempAction);
                        subs_found.append(sub);
                    }
                }
            }
            if (!subs_found.isEmpty())
                contextMenu2.addMenu(gate_menu_remove_module);
            else
                delete gate_menu_remove_module;
        }
        if (net_menu_remove_module)
        {
            QList<std::shared_ptr<module>> subs_found;
            for (const auto& sub : g_netlist->get_modules())
            {
                for (const auto& net : currently_selected_nets)
                {
                    if (sub->contains_net(net->get_ref_net()) && !subs_found.contains(sub))
                    {
                        QAction* tempAction = new QAction(QString::fromStdString(sub->get_name()) + " [ID:  " + QString::number((int)sub->get_id()) + "]", this);
                        tempAction->setData(QVariant(sub->get_id()));
                        connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_remove_nets_from_module_triggered);
                        net_menu_remove_module->addAction(tempAction);
                        subs_found.append(sub);
                    }
                }
            }
            if (!subs_found.isEmpty())
                contextMenu2.addMenu(net_menu_remove_module);
            else
                delete net_menu_remove_module;
        }
        if (all_menu_remove_module)
        {
            QList<std::shared_ptr<module>> subs_found;
            for (const auto& sub : g_netlist->get_modules())
            {
                for (const auto& net : currently_selected_nets)
                {
                    if (sub->contains_net(net->get_ref_net()) && !subs_found.contains(sub))
                    {
                        QAction* tempAction = new QAction(QString::fromStdString(sub->get_name()) + " [ID:  " + QString::number((int)sub->get_id()) + "]", this);
                        tempAction->setData(QVariant(sub->get_id()));
                        connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_remove_gates_and_nets_from_module_triggered);
                        all_menu_remove_module->addAction(tempAction);
                        subs_found.append(sub);
                    }
                }
                for (const auto& gate : currently_selected_gates)
                {
                    if (sub->contains_gate(gate->get_ref_gate()) && !subs_found.contains(sub))
                    {
                        QAction* tempAction = new QAction(QString::fromStdString(sub->get_name()) + " [ID:  " + QString::number((int)sub->get_id()) + "]", this);
                        tempAction->setData(QVariant(sub->get_id()));
                        connect(tempAction, &QAction::triggered, this, &graph_graphics_widget::handle_remove_gates_and_nets_from_module_triggered);
                        all_menu_remove_module->addAction(tempAction);
                        subs_found.append(sub);
                    }
                }
            }
            if (!subs_found.isEmpty())
                contextMenu2.addMenu(all_menu_remove_module);
            else
                delete all_menu_remove_module;
        }
        contextMenu2.addAction(&edit_name_action);
        contextMenu2.exec(mapToGlobal(pos));
    }
    else    //no item clicked
    {
        if (m_background_scene.is_grid_visible())
        {
            QAction* action = new QAction("Hide Grid", this);
            connect(action, SIGNAL(triggered()), &m_background_scene, SLOT(set_draw_grid_false()));
            contextMenu2.addAction(action);
        }
        else
        {
            QAction* action = new QAction("Show Grid", this);
            connect(action, SIGNAL(triggered()), &m_background_scene, SLOT(set_draw_grid_true()));
            contextMenu2.addAction(action);
        }
        contextMenu2.exec(mapToGlobal(pos));
    }
}

void graph_graphics_widget::edit_gate_data()
{
    current_context_gate->open_edit_name_dialog();
}

void graph_graphics_widget::edit_net_data()
{
    current_context_net->open_edit_name_dialog();
}

//this function could be integrated in the "handle_add_to_new_module_triggered" function by checking if the currently_selected_gates.size() is
//greater than 1 and then connecting the signals and slots accordingly (to either create_new_module or handle_name_for_new_module_for_multiple_gates)
void graph_graphics_widget::handle_add_gates_to_new_module_triggered()
{
    auto dialog = new input_dialog(nullptr, "Name:");
    if (dialog->exec() == QDialog::Accepted)
    {
        auto sub = g_netlist->create_module(g_netlist->get_unique_module_id(), dialog->get_text().toStdString(), g_netlist->get_top_module());
        for (const auto& gate : currently_selected_gates)
            sub->assign_gate(gate->get_ref_gate());
    }
    delete dialog;
}

void graph_graphics_widget::handle_add_gates_to_existing_module()
{
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        std::shared_ptr<module> temp = g_netlist->get_module_by_id(sender_action->data().toInt());
        if (temp)
        {
            for (const auto& gate : currently_selected_gates)
                temp->assign_gate(gate->get_ref_gate());
        }
    }
}

void graph_graphics_widget::handle_remove_gates_from_module_triggered()
{
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        std::shared_ptr<module> temp = g_netlist->get_module_by_id(sender_action->data().toInt());
        if (temp)
        {
            for (const auto& gate : currently_selected_gates)
                temp->remove_gate(gate->get_ref_gate());
        }
    }
}

void graph_graphics_widget::handle_add_nets_to_new_module_triggered()
{
    auto dialog = new input_dialog(nullptr, "Name:");
    if (dialog->exec() == QDialog::Accepted)
    {
        auto sub = g_netlist->create_module(g_netlist->get_unique_module_id(), dialog->get_text().toStdString(), g_netlist->get_top_module());
        for (const auto& net : currently_selected_nets)
            sub->assign_net(net->get_ref_net());
    }
    delete dialog;
}

void graph_graphics_widget::handle_add_nets_to_existing_module_triggered()
{
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        std::shared_ptr<module> temp = g_netlist->get_module_by_id(sender_action->data().toInt());
        if (temp)
        {
            for (const auto& net : currently_selected_nets)
                temp->assign_net(net->get_ref_net());
        }
    }
}

void graph_graphics_widget::handle_remove_nets_from_module_triggered()
{
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        std::shared_ptr<module> temp = g_netlist->get_module_by_id(sender_action->data().toInt());
        if (temp)
        {
            for (const auto& net : currently_selected_nets)
                temp->remove_net(net->get_ref_net());
        }
    }
}

void graph_graphics_widget::handle_add_gates_and_nets_to_new_module_triggered()
{
    auto dialog = new input_dialog(nullptr, "Name:");
    if (dialog->exec() == QDialog::Accepted)
    {
        auto sub = g_netlist->create_module(g_netlist->get_unique_module_id(), dialog->get_text().toStdString(), g_netlist->get_top_module());
        for (const auto& gate : currently_selected_gates)
            sub->assign_gate(gate->get_ref_gate());
        for (const auto& net : currently_selected_nets)
            sub->assign_net(net->get_ref_net());
    }
    delete dialog;
}

void graph_graphics_widget::handle_add_gates_and_nets_to_existing_module_triggered()
{
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        std::shared_ptr<module> temp = g_netlist->get_module_by_id(sender_action->data().toInt());
        if (temp)
        {
            for (auto const& gate : currently_selected_gates)
                temp->assign_gate(gate->get_ref_gate());
            for (const auto& net : currently_selected_nets)
                temp->assign_net(net->get_ref_net());
        }
    }
}

void graph_graphics_widget::handle_remove_gates_and_nets_from_module_triggered()
{
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        std::shared_ptr<module> temp = g_netlist->get_module_by_id(sender_action->data().toInt());
        if (temp)
        {
            for (const auto& gate : currently_selected_gates)
                temp->remove_gate(gate->get_ref_gate());
            for (const auto& net : currently_selected_nets)
                temp->remove_net(net->get_ref_net());
        }
    }
}

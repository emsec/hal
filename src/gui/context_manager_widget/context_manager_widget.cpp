#include "context_manager_widget/context_manager_widget.h"
#include <QVBoxLayout>
#include <QStringListModel>
#include <QStringList>
#include <QListWidgetItem>
#include <QSize>
#include <QDebug>
#include "gui_globals.h"
#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/graph_widget/graph_widget.h"
#include <QAction>
#include <QMenu>



context_manager_widget::context_manager_widget(QWidget *parent) : content_widget("Context Manager", parent), m_list_widget(new QListWidget())
{
    m_list_widget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    //load top context (top module) into list
    dynamic_context* top_context = g_graph_context_manager.get_dynamic_context("top");
    m_list_widget->addItem(top_context->name());
    m_string_to_context.insert(top_context->name(), top_context);

    m_content_layout->addWidget(m_list_widget);

    connect(m_list_widget, &QListWidget::customContextMenuRequested, this, &context_manager_widget::handle_context_menu_request);
}

void context_manager_widget::resizeEvent(QResizeEvent* event)
{
   m_list_widget->setFixedWidth(event->size().width());
}

void context_manager_widget::handle_context_menu_request(const QPoint& point)
{

    //check if right click / context menu request occured on position of an list item
    QModelIndex index = m_list_widget->indexAt(point);

    if(!index.isValid())
        return;

    //setup context menu
    QMenu context_menu;

    QAction create_action("Create New Context", &context_menu);
    QAction open_action("Open Context", &context_menu);
    QAction rename_action("Rename Context", &context_menu);
    QAction delete_action("Delete Context", &context_menu);
    
    context_menu.addAction(&create_action);
    context_menu.addAction(&open_action);
    context_menu.addAction(&rename_action);
    context_menu.addAction(&delete_action);

    connect(&create_action, &QAction::triggered, this, &context_manager_widget::handle_create_context_clicked);
    connect(&open_action, &QAction::triggered, this, &context_manager_widget::handle_open_context_clicked);
    connect(&rename_action, &QAction::triggered, this, &context_manager_widget::handle_rename_context_clicked);
    connect(&delete_action, &QAction::triggered, this, &context_manager_widget::handle_delete_context_clicked);

    //show context menu
    context_menu.exec(m_list_widget->viewport()->mapToGlobal(point));
}

void context_manager_widget::handle_create_context_clicked()
{
    //create new graph context
    m_context_counter++;
    QString new_context_name = "New Context " + QString::number(m_context_counter);
    dynamic_context* new_context = g_graph_context_manager.add_dynamic_context(new_context_name);
    m_string_to_context.insert(new_context_name, new_context);

    //default if context created from nothing -> top module + global nets (empty == better?)
    QSet<u32> global_nets;
    for (auto& net : g_netlist->get_global_inout_nets()) {
        global_nets.insert(net->get_id());
    }
    for (auto& net : g_netlist->get_global_input_nets()) {
        global_nets.insert(net->get_id());
    }
    for (auto& net : g_netlist->get_global_output_nets()) {
        global_nets.insert(net->get_id());
    }
    new_context->add(QSet<u32>{1}, QSet<u32>(), global_nets);
    
    m_list_widget->addItem(new_context_name);

    Q_EMIT context_created(new_context, new_context_name);
}

void context_manager_widget::handle_open_context_clicked()
{
    qDebug() << "Open Request";
}

void context_manager_widget::handle_rename_context_clicked()
{
    qDebug() << "Rename Request";
}

void context_manager_widget::handle_delete_context_clicked()
{
    qDebug() << "Delete Request";
}

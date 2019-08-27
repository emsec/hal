#include "context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui_globals.h"
#include <QAction>
#include <QDebug>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMenu>
#include <QSize>
#include <QStringList>
#include <QStringListModel>
#include <QVBoxLayout>

context_manager_widget::context_manager_widget(graph_tab_widget* tab_view, QWidget* parent) : content_widget("View Manager", parent), m_list_widget(new QListWidget())
{
    m_tab_view = tab_view;

    m_list_widget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    //load top context (top module) into list
    for (const auto& ctx_name : g_graph_context_manager.dynamic_context_list())
    {
        handle_context_created(g_graph_context_manager.get_dynamic_context(ctx_name));
    }

    m_content_layout->addWidget(m_list_widget);

    connect(m_list_widget, &QListWidget::customContextMenuRequested, this, &context_manager_widget::handle_context_menu_request);
    connect(m_list_widget, &QListWidget::itemDoubleClicked, this, &context_manager_widget::handle_item_double_clicked);
}

void context_manager_widget::resizeEvent(QResizeEvent* event)
{
    m_list_widget->setFixedWidth(event->size().width());
}

void context_manager_widget::handle_context_menu_request(const QPoint& point)
{
    //check if right click / context menu request occured on position of an list item
    m_clicked_index = m_list_widget->indexAt(point);
    //setup context menu
    QMenu context_menu;

    QAction create_action("Create New View", &context_menu);
    QAction open_action("Open View", &context_menu);
    QAction rename_action("Rename View", &context_menu);
    QAction delete_action("Remove View", &context_menu);

    context_menu.addAction(&create_action);
    connect(&create_action, &QAction::triggered, this, &context_manager_widget::handle_create_context_clicked);

    if (m_clicked_index.isValid())
    {
        context_menu.addAction(&open_action);
        context_menu.addAction(&rename_action);
        context_menu.addAction(&delete_action);

        connect(&open_action, &QAction::triggered, this, &context_manager_widget::handle_open_context_clicked);
        connect(&rename_action, &QAction::triggered, this, &context_manager_widget::handle_rename_context_clicked);
        connect(&delete_action, &QAction::triggered, this, &context_manager_widget::handle_delete_context_clicked);
    }
    //show context menu
    context_menu.exec(m_list_widget->viewport()->mapToGlobal(point));
}

void context_manager_widget::handle_create_context_clicked()
{
    //create new graph context
    m_context_counter++;
    QString new_context_name     = "View " + QString::number(m_context_counter);
    dynamic_context* new_context = g_graph_context_manager.add_dynamic_context(new_context_name);

    //default if context created from nothing -> top module + global nets (empty == better?)
    QSet<u32> global_nets;
    for (auto& net : g_netlist->get_global_inout_nets())
    {
        global_nets.insert(net->get_id());
    }
    for (auto& net : g_netlist->get_global_input_nets())
    {
        global_nets.insert(net->get_id());
    }
    for (auto& net : g_netlist->get_global_output_nets())
    {
        global_nets.insert(net->get_id());
    }
    new_context->add({1}, {}, global_nets);

    m_tab_view->show_context(new_context);
}

void context_manager_widget::handle_item_double_clicked(QListWidgetItem* clicked)
{
    dynamic_context* clicked_context = g_graph_context_manager.get_dynamic_context(clicked->text());
    m_tab_view->show_context(clicked_context);
}

void context_manager_widget::handle_open_context_clicked()
{
    dynamic_context* clicked_context = g_graph_context_manager.get_dynamic_context(m_clicked_index.data().toString());
    m_tab_view->show_context(clicked_context);
}

void context_manager_widget::handle_rename_context_clicked()
{
    dynamic_context* clicked_context = g_graph_context_manager.get_dynamic_context(m_clicked_index.data().toString());

    bool confirm;
    const QString new_name = QInputDialog::getText(this, "Rename view", "New name:", QLineEdit::Normal, clicked_context->name(), &confirm).trimmed();

    if (confirm && !new_name.isEmpty())
        g_graph_context_manager.rename_dynamic_context(clicked_context->name(), new_name);
}

void context_manager_widget::handle_delete_context_clicked()
{
    dynamic_context* clicked_context = g_graph_context_manager.get_dynamic_context(m_clicked_index.data().toString());
    g_graph_context_manager.remove_dynamic_context(clicked_context->name());
}

void context_manager_widget::handle_context_created(dynamic_context* context)
{
    m_list_widget->addItem(context->name());
    auto new_item = m_list_widget->item(m_list_widget->count() - 1);
    QVariant v;
    v.setValue((u64)context);
    new_item->setData(Qt::UserRole, v);
}

void context_manager_widget::handle_context_renamed(dynamic_context* context)
{
    for (int i = 0; i < m_list_widget->count(); ++i)
    {
        if (m_list_widget->item(i)->data(Qt::UserRole).value<u64>() == (u64)context)
        {
            m_list_widget->item(i)->setText(context->name());
            return;
        }
    }
}

void context_manager_widget::handle_context_removed(dynamic_context* context)
{
    for (int i = 0; i < m_list_widget->count(); ++i)
    {
        if (m_list_widget->item(i)->data(Qt::UserRole).value<u64>() == (u64)context)
        {
            delete m_list_widget->takeItem(i);    // has to be deleted manually
            return;
        }
    }
}

#include "graph_navigation_widget/old_graph_navigation_widget.h"
#include "graph_navigation_widget/old_graph_tree_model.h"
#include "graph_navigation_widget/graph_navigation_model.h"
#include "graph_navigation_widget/graph_navigation_proxy_model.h"
#include "graph_navigation_widget/navigation_filter_dialog.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QRegExp>
#include <QScrollBar>
#include <QShortcut>
#include <QTreeView>
#include <core/log.h>

#include "graph_navigation_widget/tree_navigation/tree_navigation_item.h"
#include "graph_navigation_widget/tree_navigation/tree_navigation_model.h"
#include "graph_navigation_widget/tree_navigation/tree_navigation_proxy_model.h"
#include <QSet>
#include <QShortcut>

old_graph_navigation_widget::old_graph_navigation_widget(QWidget* parent) : content_widget("Navigation", parent)
{
    m_tree_navigation_proxy_model = new tree_navigation_proxy_model(this);
    m_tree_navigation_proxy_model->setFilterKeyColumn(-1);
    m_filter_widget = new navigation_filter_widget(this);
    m_content_layout->addWidget(m_filter_widget);
    m_navigation_model      = new graph_navigation_model(this);
    m_tree_navigation_model = new tree_navigation_model(this);
    m_tree_view             = new QTreeView(this);
    m_proxy_model           = new graph_navigation_proxy_model(this);
    m_proxy_model->setSourceModel(m_navigation_model);
    m_tree_navigation_proxy_model->setSourceModel(m_tree_navigation_model);
    m_tree_view->setModel(m_tree_navigation_proxy_model);
    m_tree_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree_view->setFrameStyle(QFrame::NoFrame);
    //m_tree_view->header()->close();
    m_content_layout->addWidget(m_tree_view);
    m_content_layout->addWidget(&m_searchbar);
    m_searchbar.hide();
    m_filter_widget->hide();

    m_filter_action = new QAction("filter", this);

    m_ignore_selection_change = false;

    g_selection_relay.register_sender(this, name());

    //connect(m_filter_widget, &QLineEdit::returnPressed, this, &graph_navigation_widget::filter);
    connect(&m_searchbar, &searchbar::text_edited, this, &old_graph_navigation_widget::filter);
    connect(m_filter_action, &QAction::triggered, this, &old_graph_navigation_widget::handle_filter_action_triggered);
    connect(m_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &old_graph_navigation_widget::handle_tree_selection_changed);

    //Selection Relay Connections
    connect(&g_selection_relay, &selection_relay::selection_changed, this, &old_graph_navigation_widget::handle_selection_changed);

    //old
    connect(&g_netlist_relay, &netlist_relay::module_created, m_tree_navigation_model, &tree_navigation_model::handle_module_created);
    connect(&g_netlist_relay, &netlist_relay::module_gate_assigned, m_tree_navigation_model, &tree_navigation_model::handle_module_gate_assigned);
    connect(&g_netlist_relay, &netlist_relay::module_gate_removed, m_tree_navigation_model, &tree_navigation_model::handle_module_gate_removed);
    connect(&g_netlist_relay, &netlist_relay::module_name_changed, m_tree_navigation_model, &tree_navigation_model::handle_module_name_changed);
    connect(&g_netlist_relay, &netlist_relay::module_removed, m_tree_navigation_model, &tree_navigation_model::handle_module_removed);

    connect(&g_netlist_relay, &netlist_relay::gate_created, m_tree_navigation_model, &tree_navigation_model::handle_gate_created);
    connect(&g_netlist_relay, &netlist_relay::gate_removed, m_tree_navigation_model, &tree_navigation_model::handle_gate_removed);
    connect(&g_netlist_relay, &netlist_relay::gate_name_changed, m_tree_navigation_model, &tree_navigation_model::handle_gate_name_changed);

    connect(&g_netlist_relay, &netlist_relay::net_created, m_tree_navigation_model, &tree_navigation_model::handle_net_created);
    connect(&g_netlist_relay, &netlist_relay::net_removed, m_tree_navigation_model, &tree_navigation_model::handle_net_removed);
    connect(&g_netlist_relay, &netlist_relay::net_name_changed, m_tree_navigation_model, &tree_navigation_model::handle_net_name_changed);

    QModelIndex file_name_index = m_tree_navigation_proxy_model->index(0, 0, m_tree_view->rootIndex());
    m_tree_view->setExpanded(file_name_index, true);
    toggle_resize_columns();
}

void old_graph_navigation_widget::setup_toolbar(toolbar* toolbar){
    Q_UNUSED(toolbar)

    //toolbar->addAction(m_filter_action);
}

QList<QShortcut*> old_graph_navigation_widget::create_shortcuts()
{
    QShortcut* search_shortcut = new QShortcut(QKeySequence(tr("Ctrl+f")), this);
    connect(search_shortcut, &QShortcut::activated, this, &old_graph_navigation_widget::toggle_searchbar);

    QShortcut* resize_shortcut = new QShortcut(QKeySequence(tr("Ctrl+b")), this);
    connect(resize_shortcut, &QShortcut::activated, this, &old_graph_navigation_widget::toggle_resize_columns);

    QList<QShortcut*> list;
    list.append(search_shortcut);
    list.append(resize_shortcut);

    return list;
}

//void graph_navigation_widget::filter()
//{
//    QRegExp* regex = new QRegExp(m_filter_widget->text());
//    if (regex->isValid())
//    {
//        m_proxy_model->setFilterRegExp(*regex);
//        QString output = "navigation regular expression '" + m_filter_widget->text() + "' entered.";
//        log_info("user", output.toStdString());
//        if (!regex->isEmpty())
//            m_filter_widget->set_valid();
//        return;
//    }
//    //else
//    //m_filter_widget->set_invalid();
//}

void old_graph_navigation_widget::toggle_searchbar()
{
    if (m_searchbar.isHidden())
        m_searchbar.show();
    else
        m_searchbar.hide();
}

void old_graph_navigation_widget::filter(const QString& text)
{
    QRegExp* regex = new QRegExp(text);
    if (regex->isValid())
    {
        //m_proxy_model->setFilterRegExp(*regex);
        m_tree_navigation_proxy_model->setFilterRegExp(*regex);
        QString output = "navigation regular expression '" + text + "' entered.";
        log_info("user", output.toStdString());
    }
}

void old_graph_navigation_widget::handle_filter_action_triggered()
{
    //deprecated, use QRegularExpression instead (not supported by QSortFilterProxyModel yet)
    QRegExp* expression              = nullptr;
    navigation_filter_dialog* dialog = new navigation_filter_dialog(expression);
    dialog->exec();
    if (!expression)
        return;

    m_proxy_model->setFilterRegExp(*expression);
}

void old_graph_navigation_widget::handle_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    if (m_ignore_selection_change)
    {
        m_ignore_selection_change = false;
        return;
    }

    g_selection_relay.m_number_of_selected_gates   = 0;
    g_selection_relay.m_number_of_selected_nets    = 0;
    g_selection_relay.m_number_of_selected_modules = 0;

    QModelIndexList current_selections = selected.indexes();
    QSet<tree_navigation_item*> already_processed_items;
    for (const QModelIndex& index : current_selections)
    {
        auto item = static_cast<tree_navigation_item*>(m_tree_navigation_proxy_model->mapToSource(index).internalPointer());
        if (item && !already_processed_items.contains(item))
        {
            already_processed_items.insert(item);
            auto id = item->data(tree_navigation_model::ID_COLUMN).toInt();
            switch (item->get_type())
            {
                case tree_navigation_item::item_type::gate:
                    g_selection_relay.m_selected_gates[g_selection_relay.m_number_of_selected_gates++] = id;
                    Q_EMIT g_selection_relay.selection_changed(this);
                    return;
                case tree_navigation_item::item_type::net:
                    g_selection_relay.m_selected_nets[g_selection_relay.m_number_of_selected_nets++] = id;
                    Q_EMIT g_selection_relay.selection_changed(this);
                    return;
                case tree_navigation_item::item_type::module:
                    g_selection_relay.m_selected_modules[g_selection_relay.m_number_of_selected_modules++] = id;
                    Q_EMIT g_selection_relay.selection_changed(this);
                    return;
                default:
                    break;
            }
        }
    }
}

void old_graph_navigation_widget::handle_selection_changed(void* sender)
{
    if (sender == this)
    {
        return;
    }

    QList<u32> gate_ids, net_ids, module_ids;

    for (u32 i = 0; i < g_selection_relay.m_number_of_selected_gates; ++i)
    {
        gate_ids.append(g_selection_relay.m_selected_gates[i]);
    }
    for (u32 i = 0; i < g_selection_relay.m_number_of_selected_nets; ++i)
    {
        net_ids.append(g_selection_relay.m_selected_nets[i]);
    }
    for (u32 i = 0; i < g_selection_relay.m_number_of_selected_modules; ++i)
    {
        module_ids.append(g_selection_relay.m_selected_modules[i]);
    }

    QModelIndexList selected_indexes = m_tree_navigation_model->get_corresponding_indexes(gate_ids, net_ids, module_ids);
    QItemSelection selection;
    for (const auto& index : selected_indexes)
        selection.select(m_tree_navigation_proxy_model->mapFromSource(index), m_tree_navigation_proxy_model->mapFromSource(index));

    m_ignore_selection_change = true;
    m_tree_view->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void old_graph_navigation_widget::toggle_resize_columns()
{
    for (int i = 0; i < m_tree_navigation_model->columnCount(); i++)
        m_tree_view->resizeColumnToContents(i);
}

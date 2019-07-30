#include "module_widget/module_widget.h"

#include <core/log.h>

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include "gui/graph_manager/graph_tree_model.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/module_relay/module_relay.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMenu>
#include <QModelIndex>
#include <QRegExp>
#include <QScrollBar>
#include <QSet>
#include <QShortcut>
#include <QTreeView>
#include <QVBoxLayout>

module_widget::module_widget(QWidget* parent) : content_widget("Modules", parent),
    m_tree_view(new QTreeView(this)),
    m_module_proxy_model(new module_proxy_model(this))
{
    connect(m_tree_view, &QTreeView::customContextMenuRequested, this, &module_widget::handle_tree_view_context_menu_requested);

    m_module_proxy_model->setFilterKeyColumn(-1);
    m_module_proxy_model->setSourceModel(g_netlist_relay.get_module_model());
    m_module_proxy_model->setRecursiveFilteringEnabled(true);
    m_tree_view->setModel(m_module_proxy_model);
    m_tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree_view->setFrameStyle(QFrame::NoFrame);
    m_tree_view->header()->close();
    m_content_layout->addWidget(m_tree_view);
    m_content_layout->addWidget(&m_searchbar);
    m_searchbar.hide();

    m_filter_action = new QAction("filter", this);

    m_ignore_selection_change = false;

    g_selection_relay.register_sender(this, name());

    //connect(m_filter_widget, &QLineEdit::returnPressed, this, &graph_navigation_widget::filter);
    connect(&m_searchbar, &searchbar::text_edited, this, &module_widget::filter);
    connect(m_filter_action, &QAction::triggered, this, &module_widget::handle_filter_action_triggered);
    connect(m_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &module_widget::handle_selection_changed);
    connect(m_tree_view->selectionModel(), &QItemSelectionModel::currentChanged, this, &module_widget::handle_current_changed);

    //Selection Relay Connections
    connect(&g_selection_relay, &selection_relay::gate_selection_update, this, &module_widget::handle_gate_selection_update);
    connect(&g_selection_relay, &selection_relay::net_selection_update, this, &module_widget::handle_net_selection_update);
    connect(&g_selection_relay, &selection_relay::module_selection_update, this, &module_widget::handle_module_selection_update);
    connect(&g_selection_relay, &selection_relay::combined_selection_update, this, &module_widget::handle_combined_selection_update);

    connect(&g_selection_relay, &selection_relay::current_gate_update, this, &module_widget::handle_current_gate_update);
    connect(&g_selection_relay, &selection_relay::current_net_update, this, &module_widget::handle_current_net_update);
    connect(&g_selection_relay, &selection_relay::current_module_update, this, &module_widget::handle_current_module_update);

    connect(&g_selection_relay, &selection_relay::jump_gate_update, this, &module_widget::handle_jump_gate_update);
    connect(&g_selection_relay, &selection_relay::jump_net_update, this, &module_widget::handle_jump_net_update);
    connect(&g_selection_relay, &selection_relay::jump_module_update, this, &module_widget::handle_jump_module_update);
    connect(&g_selection_relay, &selection_relay::jump_selection_update, this, &module_widget::handle_jump_selection_update);

    connect(&g_selection_relay, &selection_relay::gate_highlight_update, this, &module_widget::handle_gate_highlight_update);
    connect(&g_selection_relay, &selection_relay::net_highlight_update, this, &module_widget::handle_net_highlight_update);
    connect(&g_selection_relay, &selection_relay::module_highlight_update, this, &module_widget::handle_module_highlight_update);
    connect(&g_selection_relay, &selection_relay::combined_highlight_update, this, &module_widget::handle_combined_highlight_update);

//    connect(&g_netlist_relay, &netlist_relay::module_event, m_tree_navigation_model, &tree_navigation_model::handle_module_event);
//    connect(&g_netlist_relay, &netlist_relay::gate_event, m_tree_navigation_model, &tree_navigation_model::handle_gate_name_changed_event);
//    connect(&g_netlist_relay, &netlist_relay::net_event, m_tree_navigation_model, &tree_navigation_model::handle_net_event);
}

void module_widget::setup_toolbar(toolbar* toolbar)
{
    Q_UNUSED(toolbar)

    //toolbar->addAction(m_filter_action);
}

QList<QShortcut*> module_widget::create_shortcuts()
{
    QShortcut* search_shortcut = new QShortcut(QKeySequence(tr("Ctrl+f")), this);
    connect(search_shortcut, &QShortcut::activated, this, &module_widget::toggle_searchbar);

    QShortcut* resize_shortcut = new QShortcut(QKeySequence(tr("Ctrl+b")), this);
    connect(resize_shortcut, &QShortcut::activated, this, &module_widget::toggle_resize_columns);

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

void module_widget::toggle_searchbar()
{
    if (m_searchbar.isHidden())
        m_searchbar.show();
    else
        m_searchbar.hide();
}

void module_widget::filter(const QString& text)
{
    QRegExp* regex = new QRegExp(text);
    if (regex->isValid())
    {
        //m_proxy_model->setFilterRegExp(*regex);
        m_module_proxy_model->setFilterRegExp(*regex);
        QString output = "navigation regular expression '" + text + "' entered.";
        log_info("user", output.toStdString());
    }
}

void module_widget::handle_tree_view_context_menu_requested(const QPoint& point)
{
    QModelIndex index = m_tree_view->indexAt(point);

    if (!index.isValid())
        return;

    QMenu context_menu;

    QAction select_action("Select Module", &context_menu);
    QAction add_selection_action("Add Selection to Module", &context_menu);
    QAction add_child_action("Add Child Module", &context_menu);
    QAction change_name_action("Change Module Name", &context_menu);
    QAction change_color_action("Change Module Color", &context_menu);
    QAction delete_action("Delete Module", &context_menu);

    context_menu.addAction(&select_action);
    context_menu.addAction(&add_selection_action);
    context_menu.addAction(&add_child_action);
    context_menu.addAction(&change_name_action);
    context_menu.addAction(&change_color_action);
    context_menu.addAction(&delete_action);

    QAction* clicked = context_menu.exec(m_tree_view->viewport()->mapToGlobal(point));

    if (!clicked)
        return;

    if (clicked == &add_selection_action)
        g_netlist_relay.debug_add_selection_to_module(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index)));

    if (clicked == &add_child_action)
        g_netlist_relay.debug_add_child_module(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index)));

    if (clicked == &change_color_action)
        g_netlist_relay.debug_change_module_color(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index)));
}

void module_widget::handle_filter_action_triggered()
{
    //deprecated, use QRegularExpression instead (not supported by QSortFilterProxyModel yet)
//    QRegExp* expression              = nullptr;
//    navigation_filter_dialog* dialog = new navigation_filter_dialog(expression);
//    dialog->exec();
//    if (!expression)
//        return;

//    m_module_proxy_model->setFilterRegExp(*expression);
}

void module_widget::handle_selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

//    if (m_ignore_selection_change)
//    {
//        m_ignore_selection_change = false;
//        return;
//    }

//    QModelIndexList current_selections = selected.indexes();
//    QSet<tree_navigation_item*> already_processed_items;
//    QList<u32> net_ids;
//    QList<u32> gate_ids;
//    QList<u32> submod_ids;
//    for (const QModelIndex& index : current_selections)
//    {
//        auto item = static_cast<tree_navigation_item*>(m_tree_navigation_proxy_model->mapToSource(index).internalPointer());
//        if (item && !already_processed_items.contains(item))
//        {
//            already_processed_items.insert(item);
//            switch (item->get_type())
//            {
//                case tree_navigation_item::item_type::gate:
//                    gate_ids.append(item->data(tree_navigation_model::ID_COLUMN).toInt());
//                    break;
//                case tree_navigation_item::item_type::net:
//                    net_ids.append(item->data(tree_navigation_model::ID_COLUMN).toInt());
//                    break;
//                case tree_navigation_item::item_type::module:
//                    submod_ids.append(item->data(tree_navigation_model::ID_COLUMN).toInt());
//                    break;
//                case tree_navigation_item::item_type::structure:
//                    break;
//                case tree_navigation_item::item_type::ignore:
//                    break;
//                default:
//                    break;
//            }
//        }
//    }
//    g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, submod_ids);
}

void module_widget::handle_current_changed(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(current)
    Q_UNUSED(previous)

//    tree_navigation_item* current_item = static_cast<tree_navigation_item*>(m_tree_navigation_proxy_model->mapToSource(current).internalPointer());
//    if (current_item)
//    {
//        switch (current_item->get_type())
//        {
//            case tree_navigation_item::item_type::gate:
//                g_selection_relay.relay_current_gate(this, current_item->data(tree_navigation_model::ID_COLUMN).toInt());
//                break;
//            case tree_navigation_item::item_type::net:
//                g_selection_relay.relay_current_net(this, current_item->data(tree_navigation_model::ID_COLUMN).toInt());
//                break;
//            case tree_navigation_item::item_type::module:
//                g_selection_relay.relay_current_module(this, current_item->data(tree_navigation_model::ID_COLUMN).toInt());
//                break;
//            case tree_navigation_item::item_type::structure:
//                break;
//            default:
//                break;
//        }
//    }
}

void module_widget::handle_gate_selection_update(void* sender, const QList<u32>& gate_ids, selection_relay::Mode mode)
{
    Q_UNUSED(sender)
    Q_UNUSED(gate_ids)
    Q_UNUSED(mode)
}

void module_widget::handle_net_selection_update(void* sender, const QList<u32>& net_ids, selection_relay::Mode mode)
{
    Q_UNUSED(sender)
    Q_UNUSED(net_ids)
    Q_UNUSED(mode)
}

void module_widget::handle_module_selection_update(void* sender, const QList<u32>& module_ids, selection_relay::Mode mode)
{
    Q_UNUSED(sender)
    Q_UNUSED(module_ids)
    Q_UNUSED(mode)
}

void module_widget::handle_combined_selection_update(void* sender, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids, selection_relay::Mode mode)
{
    Q_UNUSED(sender)
    Q_UNUSED(gate_ids)
    Q_UNUSED(net_ids)
    Q_UNUSED(module_ids)
    Q_UNUSED(mode)
//    if (sender == this)
//        return;

//    QModelIndexList selected_indexes = m_tree_navigation_model->get_corresponding_indexes(gate_ids, net_ids, module_ids);
//    QItemSelection selection;
//    for (const auto& index : selected_indexes)
//        selection.select(m_tree_navigation_proxy_model->mapFromSource(index), m_tree_navigation_proxy_model->mapFromSource(index));

//    m_ignore_selection_change = true;
//    m_tree_view->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void module_widget::handle_current_gate_update(void* sender, u32 id)
{
    Q_UNUSED(sender)
    Q_UNUSED(id)
}

void module_widget::handle_current_net_update(void* sender, u32 id)
{
    Q_UNUSED(sender)
    Q_UNUSED(id)
}

void module_widget::handle_current_module_update(void* sender, u32 id)
{
    Q_UNUSED(sender)
    Q_UNUSED(id)
}

void module_widget::handle_jump_gate_update(void* sender, u32 id)
{
    Q_UNUSED(sender)
    Q_UNUSED(id)
}

void module_widget::handle_jump_net_update(void* sender, u32 id)
{
    Q_UNUSED(sender)
    Q_UNUSED(id)
}

void module_widget::handle_jump_module_update(void* sender, u32 id)
{
    Q_UNUSED(sender)
    Q_UNUSED(id)
}

void module_widget::handle_jump_selection_update(void* sender)
{
    Q_UNUSED(sender)
}

void module_widget::handle_gate_highlight_update(void* sender, QList<u32>& ids, selection_relay::Mode mode, u32 channel)
{
    Q_UNUSED(sender)
    Q_UNUSED(ids)
    Q_UNUSED(mode)
    Q_UNUSED(channel)
}

void module_widget::handle_net_highlight_update(void* sender, QList<u32>& ids, selection_relay::Mode mode, u32 channel)
{
    Q_UNUSED(sender)
    Q_UNUSED(ids)
    Q_UNUSED(mode)
    Q_UNUSED(channel)
}

void module_widget::handle_module_highlight_update(void* sender, QList<u32>& ids, selection_relay::Mode mode, u32 channel)
{
    Q_UNUSED(sender)
    Q_UNUSED(ids)
    Q_UNUSED(mode)
    Q_UNUSED(channel)
}

void module_widget::handle_combined_highlight_update(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, selection_relay::Mode mode, u32 channel)
{
    Q_UNUSED(sender)
    Q_UNUSED(gate_ids)
    Q_UNUSED(net_ids)
    Q_UNUSED(module_ids)
    Q_UNUSED(mode)
    Q_UNUSED(channel)
}

void module_widget::toggle_resize_columns()
{
//    for (int i = 0; i < m_tree_navigation_model->columnCount(); i++)
//        m_tree_view->resizeColumnToContents(i);
}

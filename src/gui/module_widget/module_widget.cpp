#include "module_widget/module_widget.h"

#include <core/log.h>

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

//#include "gui/graph_manager/graph_tree_model.h"
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
    //m_module_proxy_model->setRecursiveFilteringEnabled(true);
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
    connect(m_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &module_widget::handle_tree_selection_changed);
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
    {
        m_searchbar.show();
        m_searchbar.setFocus();
    }
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

    QAction add_selection_action("Add Selection to Module", &context_menu);
    QAction add_child_action("Add Child Module", &context_menu);
    QAction change_name_action("Change Module Name", &context_menu);
    QAction change_color_action("Change Module Color", &context_menu);
    QAction delete_action("Delete Module", &context_menu);

    context_menu.addAction(&add_selection_action);
    context_menu.addAction(&add_child_action);
    context_menu.addAction(&change_name_action);
    context_menu.addAction(&change_color_action);
    context_menu.addAction(&delete_action);

    QAction* clicked = context_menu.exec(m_tree_view->viewport()->mapToGlobal(point));

    if (!clicked)
        return;

    if (clicked == &add_selection_action)
        g_netlist_relay.debug_add_selection_to_module(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());

    if (clicked == &add_child_action)
        g_netlist_relay.debug_add_child_module(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());

    if (clicked == &change_name_action)
        g_netlist_relay.debug_change_module_name(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());

    if (clicked == &change_color_action)
        g_netlist_relay.debug_change_module_color(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());
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

void module_widget::handle_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
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

void module_widget::toggle_resize_columns()
{
//    for (int i = 0; i < m_tree_navigation_model->columnCount(); i++)
//        m_tree_view->resizeColumnToContents(i);
}

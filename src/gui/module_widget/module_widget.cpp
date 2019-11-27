#include "module_widget/module_widget.h"

#include <core/log.h>

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

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
    m_tree_view(new module_tree_view(this)),
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

    m_ignore_selection_change = false;

    g_selection_relay.register_sender(this, name());

    connect(&m_searchbar, &searchbar::text_edited, this, &module_widget::filter);
    connect(m_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &module_widget::handle_tree_selection_changed);
}

void module_widget::setup_toolbar(toolbar* toolbar)
{
    Q_UNUSED(toolbar)
}

QList<QShortcut*> module_widget::create_shortcuts()
{
    QShortcut* search_shortcut = g_keybind_manager.make_shortcut(this, "keybinds/searchbar_toggle");
    connect(search_shortcut, &QShortcut::activated, this, &module_widget::toggle_searchbar);

    QList<QShortcut*> list;
    list.append(search_shortcut);

    return list;
}

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

    if (clicked == &add_child_action){
        g_netlist_relay.debug_add_child_module(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());
        m_tree_view->setExpanded(index, true);
    }

    if (clicked == &change_name_action)
        g_netlist_relay.debug_change_module_name(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());

    if (clicked == &change_color_action)
        g_netlist_relay.debug_change_module_color(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());

    if (clicked == &delete_action)
        g_netlist_relay.debug_delete_module(g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(index))->id());

}

void module_widget::handle_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    assert(selected.indexes().size() == 1);

//    if (m_ignore_selection_change)
//    {
//        m_ignore_selection_change = false;
//        return;
//    }

    QModelIndexList current_selections = selected.indexes();
    u32 selected_module = g_netlist_relay.get_module_model()->get_item(m_module_proxy_model->mapToSource(selected.indexes().first()))->id();

    g_selection_relay.clear();
    g_selection_relay.m_selected_modules.insert(selected_module);
    g_selection_relay.m_focus_type = selection_relay::item_type::module;
    g_selection_relay.m_focus_id = selected_module;

    g_selection_relay.relay_selection_changed(this);
}

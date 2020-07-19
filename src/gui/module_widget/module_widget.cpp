#include "module_widget/module_widget.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/module_relay/module_relay.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

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
#include <core/log.h>

namespace hal
{
    ModuleWidget::ModuleWidget(QWidget* parent) : ContentWidget("Modules", parent), m_tree_view(new ModuleTreeView(this)), m_ModuleProxyModel(new ModuleProxyModel(this))
    {
        connect(m_tree_view, &QTreeView::customContextMenuRequested, this, &ModuleWidget::handle_tree_view_context_menu_requested);

        m_ModuleProxyModel->setFilterKeyColumn(-1);
        m_ModuleProxyModel->setDynamicSortFilter(true);
        m_ModuleProxyModel->setSourceModel(g_netlist_relay.get_ModuleModel());
        //m_ModuleProxyModel->setRecursiveFilteringEnabled(true);
        m_ModuleProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_tree_view->setModel(m_ModuleProxyModel);
        m_tree_view->setSortingEnabled(true);
        m_tree_view->sortByColumn(0, Qt::AscendingOrder);
        m_tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
        m_tree_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_tree_view->setFrameStyle(QFrame::NoFrame);
        m_tree_view->header()->close();
        m_tree_view->setExpandsOnDoubleClick(false);
        m_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_tree_view->expandAll();
        m_content_layout->addWidget(m_tree_view);
        m_content_layout->addWidget(&m_searchbar);
        m_searchbar.hide();

        m_ignore_selection_change = false;

        g_selection_relay.register_sender(this, name());

        connect(&m_searchbar, &Searchbar::text_edited, this, &ModuleWidget::filter);
        connect(m_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ModuleWidget::handle_tree_selection_changed);
        connect(m_tree_view, &ModuleTreeView::doubleClicked, this, &ModuleWidget::handle_item_double_clicked);
        connect(&g_selection_relay, &SelectionRelay::selection_changed, this, &ModuleWidget::handle_selection_changed);
        connect(&g_netlist_relay, &NetlistRelay::module_submodule_removed, this, &ModuleWidget::handle_module_removed);
    }

    void ModuleWidget::setup_toolbar(Toolbar* Toolbar)
    {
        Q_UNUSED(Toolbar)
    }

    QList<QShortcut*> ModuleWidget::create_shortcuts()
    {
        QShortcut* search_shortcut = g_keybind_manager.make_shortcut(this, "keybinds/searchbar_toggle");
        connect(search_shortcut, &QShortcut::activated, this, &ModuleWidget::toggle_searchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

        return list;
    }

    void ModuleWidget::toggle_searchbar()
    {
        if (m_searchbar.isHidden())
        {
            m_searchbar.show();
            m_searchbar.setFocus();
        }
        else
            m_searchbar.hide();
    }

    void ModuleWidget::filter(const QString& text)
    {
        QRegExp* regex = new QRegExp(text);
        if (regex->isValid())
        {
            m_ModuleProxyModel->setFilterRegExp(*regex);
            QString output = "navigation regular expression '" + text + "' entered.";
            log_info("user", output.toStdString());
        }
    }

    void ModuleWidget::handle_tree_view_context_menu_requested(const QPoint& point)
    {
        QModelIndex index = m_tree_view->indexAt(point);

        if (!index.isValid())
            return;

        QMenu context_menu;

        QAction isolate_action("Isolate In New View", &context_menu);
        QAction add_selection_action("Add Graph Selection To Module", &context_menu);
        QAction add_child_action("Add Child Module", &context_menu);
        QAction change_name_action("Change Module Name", &context_menu);
        QAction change_type_action("Change Module Type", &context_menu);
        QAction change_color_action("Change Module Color", &context_menu);
        QAction delete_action("Delete Module", &context_menu);

        context_menu.addAction(&isolate_action);
        context_menu.addAction(&add_selection_action);
        context_menu.addAction(&add_child_action);
        context_menu.addAction(&change_name_action);
        context_menu.addAction(&change_type_action);
        context_menu.addAction(&change_color_action);

        u32 module_id = get_ModuleItem_from_index(index)->id();
        auto module = g_netlist->get_module_by_id(module_id);

        if(!(module == g_netlist->get_top_module()))
            context_menu.addAction(&delete_action);
 
        QAction* clicked = context_menu.exec(m_tree_view->viewport()->mapToGlobal(point));

        if (!clicked)
            return;

        if (clicked == &isolate_action)
            open_module_in_view(index);

        if (clicked == &add_selection_action)
            g_netlist_relay.debug_add_selection_to_module(get_ModuleItem_from_index(index)->id());

        if (clicked == &add_child_action)
        {
            g_netlist_relay.debug_add_child_module(get_ModuleItem_from_index(index)->id());
            m_tree_view->setExpanded(index, true);
        }

        if (clicked == &change_name_action)
            g_netlist_relay.debug_change_module_name(get_ModuleItem_from_index(index)->id());

        if (clicked == &change_type_action)
            g_netlist_relay.debug_change_module_type(get_ModuleItem_from_index(index)->id());

        if (clicked == &change_color_action)
            g_netlist_relay.debug_change_module_color(get_ModuleItem_from_index(index)->id());

        if (clicked == &delete_action)
            g_netlist_relay.debug_delete_module(get_ModuleItem_from_index(index)->id());
    }

    void ModuleWidget::handle_module_removed(std::shared_ptr<Module> module, u32 module_id)
    {
        UNUSED(module);
        UNUSED(module_id);

        //prevents the execution of "handle_tree_selection_changed"
        //when a module is (re)moved the corresponding item in the tree is deleted and deselected, thus also triggering "handle_tree_selection_changed"
        //this call due to the selection model signals is unwanted behavior because "handle_tree_selection_changed" is ment to only react to an "real" action performed by the user on the tree itself
        m_ignore_selection_change = true;
    }

    void ModuleWidget::handle_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
    {
        Q_UNUSED(selected)
        Q_UNUSED(deselected)

        if (m_ignore_selection_change || g_netlist_relay.get_ModuleModel()->is_modifying())
            return;

        g_selection_relay.clear();

        QModelIndexList current_selection = m_tree_view->selectionModel()->selectedIndexes();

        for (const auto& index : current_selection)
        {
            u32 module_id = get_ModuleItem_from_index(index)->id();
            g_selection_relay.m_selected_modules.insert(module_id);
        }

        if (current_selection.size() == 1)
        {
            g_selection_relay.m_focus_type = SelectionRelay::item_type::module;
            g_selection_relay.m_focus_id   = g_netlist_relay.get_ModuleModel()->get_item(m_ModuleProxyModel->mapToSource(current_selection.first()))->id();
        }

        g_selection_relay.relay_selection_changed(this);
    }

    void ModuleWidget::handle_item_double_clicked(const QModelIndex& index)
    {
        open_module_in_view(index);
    }

    void ModuleWidget::open_module_in_view(const QModelIndex& index)
    {
        auto module = g_netlist->get_module_by_id(get_ModuleItem_from_index(index)->id());

        if (!module)
            return;

        GraphContext* new_context = nullptr;
        new_context                = g_graph_context_manager.create_new_context(QString::fromStdString(module->get_name()));
        new_context->add({module->get_id()}, {});
    }

    void ModuleWidget::handle_selection_changed(void* sender)
    {
        if (sender == this)
            return;

        m_ignore_selection_change = true;

        QItemSelection module_selection;

        for (auto module_id : g_selection_relay.m_selected_modules)
        {
            QModelIndex index = m_ModuleProxyModel->mapFromSource(g_netlist_relay.get_ModuleModel()->get_index(g_netlist_relay.get_ModuleModel()->get_item(module_id)));
            module_selection.select(index, index);
        }

        m_tree_view->selectionModel()->select(module_selection, QItemSelectionModel::SelectionFlag::ClearAndSelect);

        m_ignore_selection_change = false;
    }

    ModuleItem* ModuleWidget::get_ModuleItem_from_index(const QModelIndex& index)
    {
        return g_netlist_relay.get_ModuleModel()->get_item(m_ModuleProxyModel->mapToSource(index));
    }
}

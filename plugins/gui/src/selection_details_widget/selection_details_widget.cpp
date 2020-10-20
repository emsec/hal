#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/gate_details_widget.h"
#include "gui/selection_details_widget/net_details_widget.h"
#include "gui/selection_details_widget/module_details_widget.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/selection_history_navigator/selection_history_navigator.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "gui/searchbar/searchbar.h"
#include "gui/toolbar/toolbar.h"
#include "gui/gui_utils/graphics.h"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QSplitter>
#include <QListWidget>
#include <QMenu>
#include <QAction>

const QString ADD_TO_GROUPING("Add to grouping ");

namespace hal
{
    SelectionDetailsWidget::SelectionDetailsWidget(QWidget* parent)
        : ContentWidget("Selection Details", parent), m_numberSelectedItems(0),
          m_restoreLastSelection(new QAction),
          m_selectionToGrouping(new QAction),
          m_selectionToModule(new QAction),
          m_search_action(new QAction),
          m_history(new SelectionHistoryNavigator(5))
    {
        //needed to load the properties
        ensurePolished();

        m_splitter = new QSplitter(Qt::Horizontal, this);
        //m_splitter->setStretchFactor(0,5); /* Doesn't do anything? */
        //m_splitter->setStretchFactor(1,10);

        //container for left side of splitter containing a selection tree view and a searchbar
        QWidget* treeViewContainer = new QWidget(m_splitter);
        //treeViewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); /*Does not work, but should?*/
        treeViewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); /*Kinda works? Does not give as much space as previous implementation without container.*/

        QVBoxLayout* containerLayout = new QVBoxLayout(treeViewContainer);

        m_selectionTreeView  = new SelectionTreeView(treeViewContainer);
        m_selectionTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_selectionTreeView->setMinimumWidth(280);
        m_selectionTreeView->hide();

        m_searchbar = new Searchbar(treeViewContainer);
        m_searchbar->hide();

        containerLayout->addWidget(m_selectionTreeView);
        containerLayout->addWidget(m_searchbar);
        containerLayout->setSpacing(0);
        containerLayout->setContentsMargins(0,0,0,0);


        m_selectionDetails = new QWidget(m_splitter);
        QVBoxLayout* selDetailsLayout = new QVBoxLayout(m_selectionDetails);

        m_stacked_widget = new QStackedWidget(m_selectionDetails);

        m_gate_details = new GateDetailsWidget(m_selectionDetails);
        m_stacked_widget->addWidget(m_gate_details);

        m_net_details = new NetDetailsWidget(m_selectionDetails);
        m_stacked_widget->addWidget(m_net_details);

        m_module_details = new ModuleDetailsWidget(this);
        m_stacked_widget->addWidget(m_module_details);

        m_item_deleted_label = new QLabel(m_selectionDetails);
        m_item_deleted_label->setText("Currently selected item has been removed. Please consider relayouting the Graph.");
        m_item_deleted_label->setWordWrap(true);
        m_item_deleted_label->setAlignment(Qt::AlignmentFlag::AlignTop);
        m_stacked_widget->addWidget(m_item_deleted_label);

        m_no_selection_label = new QLabel(m_selectionDetails);
        m_no_selection_label->setText("No Selection");
        m_no_selection_label->setWordWrap(true);
        m_no_selection_label->setAlignment(Qt::AlignmentFlag::AlignCenter);
        m_stacked_widget->addWidget(m_no_selection_label);

        m_stacked_widget->setCurrentWidget(m_no_selection_label);

        selDetailsLayout->addWidget(m_stacked_widget);

        m_content_layout->addWidget(m_splitter);

        //    m_table_widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        //    m_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //    m_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
        //    m_table_widget->setSelectionMode(QAbstractItemView::NoSelection);
        //    m_table_widget->setShowGrid(false);
        //    m_table_widget->setAlternatingRowColors(true);
        //    m_table_widget->horizontalHeader()->setStretchLastSection(true);
        //    m_table_widget->viewport()->setFocusPolicy(Qt::NoFocus);

        m_restoreLastSelection->setToolTip("Restore last selection");
        m_selectionToGrouping->setToolTip("Assign to grouping");
        m_selectionToModule->setToolTip("Move to module");
        m_selectionToGrouping->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_to_grouping_icon_path));
        m_selectionToModule->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_to_module_icon_path));
        canRestoreSelection();
        canMoveToModule(0);

        m_search_action->setToolTip("Search");
        enableSearchbar(false);  // enable upon first non-zero selection
        m_selectionToGrouping->setDisabled(true);
        m_selectionToModule->setDisabled(true);

        connect(m_restoreLastSelection, &QAction::triggered, this, &SelectionDetailsWidget::restoreLastSelection);
        connect(m_selectionToGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToGrouping);
        connect(m_selectionToModule, &QAction::triggered, this, &SelectionDetailsWidget::selectionToModule);
        connect(m_search_action, &QAction::triggered, this, &SelectionDetailsWidget::toggle_searchbar);
        connect(m_selectionTreeView, &SelectionTreeView::triggerSelection, this, &SelectionDetailsWidget::handleTreeSelection);
        connect(g_selection_relay, &SelectionRelay::selection_changed, this, &SelectionDetailsWidget::handle_selection_update);
        connect(m_searchbar, &Searchbar::text_edited, m_selectionTreeView, &SelectionTreeView::handle_filter_text_changed);
        connect(m_searchbar, &Searchbar::text_edited, this, &SelectionDetailsWidget::handle_filter_text_changed);
    }

    void SelectionDetailsWidget::restoreLastSelection()
    {
        g_selection_relay->clear();
        m_history->restorePreviousEntry();
        g_selection_relay->relay_selection_changed(nullptr);
        canRestoreSelection();
    }

    void SelectionDetailsWidget::selectionToModule()
    {
        if (g_selection_relay->m_selected_modules.size() + g_selection_relay->m_selected_modules.size() <= 0) return;
        QMenu* menu = new QMenu(this);
        QAction* action = menu->addAction("New module …");
//        QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_move_new_action);
        menu->addSeparator();

        QActionGroup* moduleAction = new QActionGroup(menu);
        for (Module* module : g_netlist->get_modules())
        {
            bool canAdd = true;

            for (u32 mid : g_selection_relay->m_selected_modules)
            {
                Module* m = g_netlist->get_module_by_id(mid);
                if (!m) continue;
                if (module == m || module->contains_module(m))
                {
                    canAdd = false;
                    break;
                }
            }
            if (canAdd)
                for (u32 gid : g_selection_relay->m_selected_gates)
                {
                    Gate* g = g_netlist->get_gate_by_id(gid);
                    if (!g) continue;
                    if (module->contains_gate(g))
                    {
                        canAdd = false;
                        break;
                    }
                }
            // don't allow a gate to be moved into its current module
            // && don't allow a module to be moved into its current module
            // && don't allow a module to be moved into itself
            // (either check automatically passes if g respective m is nullptr, so we
            // don't have to create two loops)
            if (canAdd)
            {
                QString modName = QString::fromStdString(module->get_name());
                const u32 modId = module->get_id();
                action          = menu->addAction(modName);
                moduleAction->addAction(action);
                action->setData(modId);
            }
        }
       // QObject::connect(module_actions, SIGNAL(triggered(QAction*)), this, SLOT(handle_move_action(QAction*)));
        menu->exec(mapToGlobal(geometry().topRight()));
    }

    void SelectionDetailsWidget::selectionToGrouping()
    {
        QStringList groupingNames =
                g_content_manager->getGroupingManagerWidget()->getModel()->groupingNames();
        if (groupingNames.isEmpty())
            selectionToNewGrouping();
        else
        {
            QMenu* contextMenu = new QMenu(this);

            QAction* newGrouping = contextMenu->addAction("Create new grouping from selected items");
            connect(newGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToNewGrouping);

            contextMenu->addSeparator();

            for (const QString& gn : groupingNames)
            {
                QAction* toGrouping = contextMenu->addAction(ADD_TO_GROUPING+gn);
                connect(toGrouping, &QAction::triggered, this, &SelectionDetailsWidget::selectionToExistingGrouping);
            }
            contextMenu->exec(mapToGlobal(geometry().topLeft()+QPoint(100,0)));
        }
    }

    void SelectionDetailsWidget::selectionToNewGrouping()
    {
        Grouping* grp = g_content_manager->getGroupingManagerWidget()->getModel()->addDefaultEntry();
        if (grp) selectionToGroupingInternal(grp);
    }

    void SelectionDetailsWidget::selectionToExistingGrouping()
    {
        const QAction* action = static_cast<const QAction*>(QObject::sender());
        QString grpName = action->text();
        if (grpName.startsWith(ADD_TO_GROUPING)) grpName.remove(0,ADD_TO_GROUPING.size());
        Grouping* grp =
                g_content_manager->getGroupingManagerWidget()->getModel()->groupingByName(grpName);
        if (grp) selectionToGroupingInternal(grp);
    }

    void SelectionDetailsWidget::selectionToGroupingInternal(Grouping* grp)
    {
        for (u32 mid : g_selection_relay->m_selected_modules)
        {
            Module* m = g_netlist->get_module_by_id(mid);
            if (m)
            {
                Grouping* mg = m->get_grouping();
                if (mg) mg->remove_module(m);
                grp->assign_module(m);
            }
        }
        for (u32 gid : g_selection_relay->m_selected_gates)
        {
            Gate* g = g_netlist->get_gate_by_id(gid);
            if (g)
            {
                Grouping* gg = g->get_grouping();
                if (gg) gg->remove_gate(g);
                grp->assign_gate(g);
            }
        }
        for (u32 nid : g_selection_relay->m_selected_nets)
        {
            Net* n = g_netlist->get_net_by_id(nid);
            if (n)
            {
                Grouping* ng = n->get_grouping();
                if (ng) ng->remove_net(n);
                grp->assign_net(n);
            }
        }
        g_selection_relay->clear();
        g_selection_relay->relay_selection_changed(nullptr);
        canRestoreSelection();
    }

    void SelectionDetailsWidget::enableSearchbar(bool enable)
    {
        QString iconStyle = enable
                ? m_search_icon_style
                : m_disabled_icon_style;
        m_search_action->setIcon(gui_utility::get_styled_svg_icon(iconStyle, m_search_icon_path));
        if (!enable && m_searchbar->isVisible())
        {
            m_searchbar->hide();
            setFocus();
        }
        m_search_action->setEnabled(enable);
    }

    void SelectionDetailsWidget::canRestoreSelection()
    {
        bool enable = m_history->hasPreviousEntry();

        QString iconStyle = enable
                ? m_search_icon_style
                : m_disabled_icon_style;

        m_restoreLastSelection->setIcon(gui_utility::get_styled_svg_icon(iconStyle, m_restore_icon_path));
        m_restoreLastSelection->setEnabled(enable);
    }

    void SelectionDetailsWidget::canMoveToModule(int nodes)
    {
        QString iconStyle = nodes > 0
                ? m_to_module_icon_style
                : m_disabled_icon_style;
        m_selectionToModule->setIcon(gui_utility::get_styled_svg_icon(iconStyle, m_to_module_icon_path));
        m_selectionToModule->setEnabled(nodes > 0);
    }

    void SelectionDetailsWidget::handle_selection_update(void* sender)
    {
        //called update methods with id = 0 to reset widget to the internal state of not updating because its not visible
        //when all details widgets are finished maybe think about more elegant way

        if (sender == this)
        {
            return;
        }

        SelectionTreeProxyModel* proxy = static_cast<SelectionTreeProxyModel*>(m_selectionTreeView->model());
        if (proxy->isGraphicsBusy()) return;

        m_searchbar->clear();
        proxy->handle_filter_text_changed(QString());
        handle_filter_text_changed(QString());

        m_numberSelectedItems = g_selection_relay->m_selected_gates.size() + g_selection_relay->m_selected_modules.size() + g_selection_relay->m_selected_nets.size();
        QVector<const SelectionTreeItem*> defaultHighlight;

        if (m_numberSelectedItems)
        {
            // more than 1 item selected, populate and make visible
            m_selectionTreeView->populate(true);
            defaultHighlight.append(m_selectionTreeView->itemFromIndex());

            m_history->storeCurrentSelection();
            canRestoreSelection();
            canMoveToModule(g_selection_relay->m_selected_gates.size() + g_selection_relay->m_selected_modules.size());
            enableSearchbar(true);
            m_selectionToGrouping->setEnabled(true);
            m_selectionToModule->setEnabled(true);
            m_selectionToGrouping->setIcon(gui_utility::get_styled_svg_icon(m_to_grouping_icon_style, m_to_grouping_icon_path));
            m_selectionToModule->setIcon(gui_utility::get_styled_svg_icon(m_to_module_icon_style, m_to_module_icon_path));
        }
        else
        {
            // nothing selected
            singleSelectionInternal(nullptr);
            // clear and hide tree
            m_selectionTreeView->populate(false);
            m_history->emptySelection();
            canMoveToModule(0);
            enableSearchbar(false);
            m_selectionToGrouping->setDisabled(true);
            m_selectionToModule->setDisabled(true);
            m_selectionToGrouping->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_to_grouping_icon_path));
            m_selectionToModule->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_to_module_icon_path));

            return;
        }


        if (!g_selection_relay->m_selected_modules.isEmpty())
        {
            SelectionTreeItemModule sti(*g_selection_relay->m_selected_modules.begin());
            singleSelectionInternal(&sti);
        }
        else if (!g_selection_relay->m_selected_gates.isEmpty())
        {
            SelectionTreeItemGate sti(*g_selection_relay->m_selected_gates.begin());
            singleSelectionInternal(&sti);
        }
        else if (!g_selection_relay->m_selected_nets.isEmpty())
        {
            SelectionTreeItemNet sti(*g_selection_relay->m_selected_nets.begin());
            singleSelectionInternal(&sti);
        }
        Q_EMIT triggerHighlight(defaultHighlight);
    }

    void SelectionDetailsWidget::handleTreeSelection(const SelectionTreeItem *sti)
    {
        singleSelectionInternal(sti);
        QVector<const SelectionTreeItem*> highlight;
        if (sti) highlight.append(sti);
        Q_EMIT triggerHighlight(highlight);
    }

    void SelectionDetailsWidget::singleSelectionInternal(const SelectionTreeItem *sti)
    {
        SelectionTreeItem::itemType_t tp = sti
                ? sti->itemType()
                : SelectionTreeItem::NullItem;

        switch (tp) {
        case SelectionTreeItem::NullItem:
            m_module_details->update(0);
            m_stacked_widget->setCurrentWidget(m_no_selection_label);
//            set_name("Selection Details");
            break;
        case SelectionTreeItem::ModuleItem:
            m_module_details->update(sti->id());
            m_stacked_widget->setCurrentWidget(m_module_details);
//            if (m_numberSelectedItems==1) set_name("Module Details");
            break;
        case SelectionTreeItem::GateItem:
            m_module_details->update(0);
            m_gate_details->update(sti->id());
            m_stacked_widget->setCurrentWidget(m_gate_details);
//            if (m_numberSelectedItems==1) set_name("Gate Details");
            break;
        case SelectionTreeItem::NetItem:
            m_module_details->update(0);
            m_net_details->update(sti->id());
            m_stacked_widget->setCurrentWidget(m_net_details);
//            if (m_numberSelectedItems==1) set_name("Net Details");
            break;
        default:
            break;
        }
    }

    QList<QShortcut *> SelectionDetailsWidget::create_shortcuts()
    {
        QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"),this);
        connect(search_shortcut, &QShortcut::activated, this, &SelectionDetailsWidget::toggle_searchbar);

        return (QList<QShortcut*>() << search_shortcut);
    }

    void SelectionDetailsWidget::toggle_searchbar()
    {
        if(m_searchbar->isHidden())
        {
            m_searchbar->show();
            m_searchbar->setFocus();
        }
        else
        {
            m_searchbar->hide();
            setFocus();
        }
    }

    void SelectionDetailsWidget::handle_filter_text_changed(const QString& filter_text)
    {
        if(filter_text.isEmpty())
            m_search_action->setIcon(gui_utility::get_styled_svg_icon(m_search_icon_style, m_search_icon_path));
        else
            m_search_action->setIcon(gui_utility::get_styled_svg_icon(m_search_active_icon_style, m_search_icon_path));
    }

    void SelectionDetailsWidget::setup_toolbar(Toolbar* toolbar)
    {
        toolbar->addAction(m_restoreLastSelection);
        toolbar->addAction(m_selectionToGrouping);
        toolbar->addAction(m_selectionToModule);
        toolbar->addAction(m_search_action);
    }

    QString SelectionDetailsWidget::disabled_icon_style() const
    {
        return m_disabled_icon_style;
    }

    void SelectionDetailsWidget::set_disabled_icon_style(const QString& style)
    {
        m_disabled_icon_style = style;
    }

    QString SelectionDetailsWidget::search_icon_path() const
    {
        return m_search_icon_path;
    }

    QString SelectionDetailsWidget::search_icon_style() const
    {
        return m_search_icon_style;
    }

    QString SelectionDetailsWidget::search_active_icon_style() const
    {
        return m_search_active_icon_style;
    }

    void SelectionDetailsWidget::set_search_icon_path(const QString& path)
    {
        m_search_icon_path = path;
    }

    void SelectionDetailsWidget::set_search_icon_style(const QString& style)
    {
        m_search_icon_style = style;
    }

    void SelectionDetailsWidget::set_search_active_icon_style(const QString& style)
    {
        m_search_active_icon_style = style;
    }

    QString SelectionDetailsWidget::restore_icon_path() const
    {
        return m_restore_icon_path;
    }

    QString SelectionDetailsWidget::restore_icon_style() const
    {
        return m_restore_icon_style;
    }

    void SelectionDetailsWidget::set_restore_icon_path(const QString& path)
    {
        m_restore_icon_path = path;
    }

    void SelectionDetailsWidget::set_restore_icon_style(const QString& style)
    {
        m_restore_icon_style = style;
    }
    
    QString SelectionDetailsWidget::to_grouping_icon_path() const
    {
        return m_to_grouping_icon_path;
    }

    QString SelectionDetailsWidget::to_grouping_icon_style() const
    {
        return m_to_grouping_icon_style;
    }

    void SelectionDetailsWidget::set_to_grouping_icon_path(const QString& path)
    {
        m_to_grouping_icon_path = path;
    }

    void SelectionDetailsWidget::set_to_grouping_icon_style(const QString& style)
    {
        m_to_grouping_icon_style = style;
    }
    
    QString SelectionDetailsWidget::to_module_icon_path() const
    {
        return m_to_module_icon_path;
    }
    
    QString SelectionDetailsWidget::to_module_icon_style() const
    {
        return m_to_module_icon_style;
    }
    
    void SelectionDetailsWidget::set_to_module_icon_path(const QString& path)
    {
        m_to_module_icon_path = path;
    }
    
    void SelectionDetailsWidget::set_to_module_icon_style(const QString& style)
    {
        m_to_module_icon_style = style;
    }
}

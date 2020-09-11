#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/gate_details_widget.h"
#include "gui/selection_details_widget/net_details_widget.h"
#include "gui/selection_details_widget/module_details_widget.h"
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

namespace hal
{
    const QString SelectionDetailsWidget::disableIconStyle = QString("all->#515050");


    SelectionDetailsWidget::SelectionDetailsWidget(QWidget* parent)
        : ContentWidget("Selection Details", parent), m_numberSelectedItems(0),
          m_restoreLastSelection(new QAction),
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
        canRestoreSelection();

        m_search_action->setToolTip("Search");
        enableSearchbar(false);  // enable upon first non-zero selection

        connect(m_restoreLastSelection, &QAction::triggered, this, &SelectionDetailsWidget::restoreLastSelection);
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

    void SelectionDetailsWidget::enableSearchbar(bool enable)
    {
        QString iconStyle = enable
                ? m_search_icon_style
                : disableIconStyle;
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
                : disableIconStyle;
        QString iconName(":/icons/undo2");

        m_restoreLastSelection->setIcon( gui_utility::get_styled_svg_icon(iconStyle,iconName));
        m_restoreLastSelection->setEnabled(enable);
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
            enableSearchbar(true);
        }
        else
        {
            // nothing selected
            singleSelectionInternal(nullptr);
            // clear and hide tree
            m_selectionTreeView->populate(false);
            m_history->emptySelection();
            enableSearchbar(false);
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
            m_search_action->setIcon(gui_utility::get_styled_svg_icon("all->#30ac4f", m_search_icon_path)); //color test, integrate into stylsheet later
    }

    void SelectionDetailsWidget::setup_toolbar(Toolbar* toolbar)
    {
        toolbar->addAction(m_restoreLastSelection);
        toolbar->addAction(m_search_action);
    }

    QString SelectionDetailsWidget::search_icon_path() const
    {
        return m_search_icon_path;
    }

    QString SelectionDetailsWidget::search_icon_style() const
    {
        return m_search_icon_style;
    }

    void SelectionDetailsWidget::set_search_icon_path(const QString& path)
    {
        m_search_icon_path = path;
    }

    void SelectionDetailsWidget::set_search_icon_style(const QString& style)
    {
        m_search_icon_style = style;
    }    
}

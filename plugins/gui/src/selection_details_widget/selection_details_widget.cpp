#include "selection_details_widget/selection_details_widget.h"
#include "selection_details_widget/tree_navigation/selection_tree_view.h"
#include "selection_details_widget/gate_details_widget.h"
#include "selection_details_widget/net_details_widget.h"
#include "selection_details_widget/module_details_widget.h"

#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "searchbar/searchbar.h"

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
    SelectionDetailsWidget::SelectionDetailsWidget(QWidget* parent)
        : ContentWidget("Details", parent), m_numberSelectedItems(0)
    {
        m_splitter = new QSplitter(Qt::Horizontal, this);
        //m_splitter->setStretchFactor(0,5); /* Doesn't do anything? */ 
        //m_splitter->setStretchFactor(1,10);

        //container for left side of splitter containing a selection tree view and a searchbar
        QWidget* treeViewContainer = new QWidget(m_splitter);
        //treeViewContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); /*Does not work, but should?*/
        treeViewContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding); /*Kinda works? Does not give as much space as previous implementation without container.*/

        QVBoxLayout* containerLayout = new QVBoxLayout(treeViewContainer);

        m_selectionTreeView  = new SelectionTreeView(treeViewContainer);
        m_searchbar = new Searchbar(treeViewContainer);
        m_searchbar->hide();
      
        containerLayout->addWidget(m_selectionTreeView);
        containerLayout->addWidget(m_searchbar);
        containerLayout->setSpacing(0);
        containerLayout->setContentsMargins(0,0,0,0);


        m_selectionDetails = new QWidget(m_splitter);
        QVBoxLayout* selDetailsLayout = new QVBoxLayout(m_selectionDetails);

        m_stacked_widget = new QStackedWidget(m_selectionDetails);

        m_empty_widget = new QWidget(m_selectionDetails);
        m_stacked_widget->addWidget(m_empty_widget);

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

        m_stacked_widget->setCurrentWidget(m_empty_widget);

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

        connect(m_selectionTreeView, &SelectionTreeView::triggerSelection, this, &SelectionDetailsWidget::handleTreeSelection);

        connect(&g_selection_relay, &SelectionRelay::selection_changed, this, &SelectionDetailsWidget::handle_selection_update);
        m_selectionTreeView->hide();
        set_name("Selection Details");
    }

    void SelectionDetailsWidget::handle_selection_update(void* sender)
    {
        //called update methods with id = 0 to reset widget to the internal state of not updating because its not visible
        //when all details widgets are finished maybe think about more elegant way

        if (sender == this)
        {
            return;
        }

        m_numberSelectedItems = g_selection_relay.m_selected_gates.size() + g_selection_relay.m_selected_modules.size() + g_selection_relay.m_selected_nets.size();
        QVector<const SelectionTreeItem*> defaultHighlight;
        const SelectionTreeModel* treeModel;

        switch (m_numberSelectedItems) {
        case 0:
        {
            singleSelectionInternal(nullptr);
            // clear and hide tree
            m_selectionTreeView->populate(false);
//            set_name("Selection Details (nothing selected)");
            return;
        }
// executive decision: treat single selected item the same way as multiple
//        case 1:
//            m_selectionTreeView->populate(false);
//            break;
        default:
            // more than 1 item selected, populate and make visible
            m_selectionTreeView->populate(true);
            treeModel = static_cast<const SelectionTreeModel*>(m_selectionTreeView->model());
            defaultHighlight.append(treeModel->itemFromIndex(treeModel->defaultIndex()));
            break;
        }

        if (!g_selection_relay.m_selected_modules.isEmpty())
        {
            SelectionTreeItemModule sti(*g_selection_relay.m_selected_modules.begin());
            singleSelectionInternal(&sti);
        }
        else if (!g_selection_relay.m_selected_gates.isEmpty())
        {
            SelectionTreeItemGate sti(*g_selection_relay.m_selected_gates.begin());
            singleSelectionInternal(&sti);
        }
        else if (!g_selection_relay.m_selected_nets.isEmpty())
        {
            SelectionTreeItemNet sti(*g_selection_relay.m_selected_nets.begin());
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
            m_stacked_widget->setCurrentWidget(m_empty_widget);
//            set_name("Selection Details");
            break;
        case SelectionTreeItem::ModuleItem:
            m_module_details->update(sti->id());
            m_stacked_widget->setCurrentWidget(m_module_details);
//            if (m_numberSelectedItems==1) set_name("Module Details");
            break;
        case SelectionTreeItem::GateItem:
//            m_searchbar->hide();
            m_module_details->update(0);
            m_gate_details->update(sti->id());
            m_stacked_widget->setCurrentWidget(m_gate_details);
//            if (m_numberSelectedItems==1) set_name("Gate Details");
            break;
        case SelectionTreeItem::NetItem:
//            m_searchbar->hide();
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
}

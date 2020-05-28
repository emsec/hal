#include "selection_details_widget/selection_details_widget.h"

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

selection_details_widget::selection_details_widget(QWidget* parent) : content_widget("Details", parent)
{
    m_stacked_widget = new QStackedWidget(this);

    m_empty_widget = new QWidget(this);
    m_stacked_widget->addWidget(m_empty_widget);

    m_gate_details = new gate_details_widget(this);
    m_stacked_widget->addWidget(m_gate_details);

    m_net_details = new net_details_widget(this);
    m_stacked_widget->addWidget(m_net_details);

    m_module_details = new module_details_widget(this);
    m_stacked_widget->addWidget(m_module_details);

    m_item_deleted_label = new QLabel(this);
    m_item_deleted_label->setText("Currently selected item has been removed. Please consider relayouting the Graph.");
    m_item_deleted_label->setWordWrap(true);
    m_item_deleted_label->setAlignment(Qt::AlignmentFlag::AlignTop);
    m_stacked_widget->addWidget(m_item_deleted_label);

    m_stacked_widget->setCurrentWidget(m_empty_widget);

    m_searchbar = new searchbar(this);
    m_searchbar->hide();

    m_content_layout->addWidget(m_stacked_widget);
    m_content_layout->addWidget(m_searchbar);

    //    m_table_widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //    m_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //    m_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //    m_table_widget->setSelectionMode(QAbstractItemView::NoSelection);
    //    m_table_widget->setShowGrid(false);
    //    m_table_widget->setAlternatingRowColors(true);
    //    m_table_widget->horizontalHeader()->setStretchLastSection(true);
    //    m_table_widget->viewport()->setFocusPolicy(Qt::NoFocus);

    connect(&g_selection_relay, &selection_relay::selection_changed, this, &selection_details_widget::handle_selection_update);
}

void selection_details_widget::handle_selection_update(void* sender)
{
    if (sender == this)
    {
        return;
    }

    if(g_selection_relay.m_selected_gates.isEmpty() &&
       g_selection_relay.m_selected_modules.isEmpty() &&
       g_selection_relay.m_selected_nets.isEmpty())
    {
        m_stacked_widget->setCurrentWidget(m_empty_widget);
    }

    if (!g_selection_relay.m_selected_modules.isEmpty())
    {
        m_module_details->update(*g_selection_relay.m_selected_modules.begin());
        m_stacked_widget->setCurrentWidget(m_module_details);
    }
    else if (!g_selection_relay.m_selected_gates.isEmpty())
    {
        m_searchbar->hide();
        m_gate_details->update(*g_selection_relay.m_selected_gates.begin());
        m_stacked_widget->setCurrentWidget(m_gate_details);
    }
    else if (!g_selection_relay.m_selected_nets.isEmpty())
    {
        m_searchbar->hide();
        m_net_details->update(*g_selection_relay.m_selected_nets.begin());
        m_stacked_widget->setCurrentWidget(m_net_details);
    }
}

QList<QShortcut *> selection_details_widget::create_shortcuts()
{
    QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"),this);
    connect(search_shortcut, &QShortcut::activated, this, &selection_details_widget::toggle_searchbar);

    return (QList<QShortcut*>() << search_shortcut);
}

void selection_details_widget::toggle_searchbar()
{
    if(m_stacked_widget->currentWidget() != m_module_details)
        return;

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

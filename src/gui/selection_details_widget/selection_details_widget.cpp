#include "selection_details_widget/selection_details_widget.h"

#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

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

    m_content_layout->addWidget(m_stacked_widget);

    //    m_table_widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //    m_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //    m_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //    m_table_widget->setSelectionMode(QAbstractItemView::NoSelection);
    //    m_table_widget->setShowGrid(false);
    //    m_table_widget->setAlternatingRowColors(true);
    //    m_table_widget->horizontalHeader()->setStretchLastSection(true);
    //    m_table_widget->viewport()->setFocusPolicy(Qt::NoFocus);

    connect(&g_selection_relay, &selection_relay::current_gate_update, this, &selection_details_widget::handle_current_gate_update);
    connect(&g_selection_relay, &selection_relay::current_net_update, this, &selection_details_widget::handle_current_net_update);
    connect(&g_selection_relay, &selection_relay::current_module_update, this, &selection_details_widget::handle_current_module_update);
    connect(&g_selection_relay, &selection_relay::current_cleared_update, this, &selection_details_widget::handle_current_cleared_update);
    connect(&g_selection_relay, &selection_relay::current_deleted_update, this, &selection_details_widget::handle_current_deleted_update);
}

void selection_details_widget::handle_current_gate_update(void* sender, u32 id)
{
    Q_UNUSED(sender)

    m_gate_details->update(id);
    m_stacked_widget->setCurrentWidget(m_gate_details);
}

void selection_details_widget::handle_current_net_update(void* sender, u32 id)
{
    Q_UNUSED(sender)

    m_net_details->update(id);
    m_stacked_widget->setCurrentWidget(m_net_details);
}

void selection_details_widget::handle_current_module_update(void* sender, u32 id)
{
    Q_UNUSED(sender)

    m_module_details->update(id);
    m_stacked_widget->setCurrentWidget(m_module_details);
}

void selection_details_widget::handle_current_cleared_update(void* sender)
{
    Q_UNUSED(sender)

    m_stacked_widget->setCurrentWidget(m_empty_widget);
}

void selection_details_widget::handle_current_deleted_update(void* sender)
{
    Q_UNUSED(sender)

    m_stacked_widget->setCurrentWidget(m_item_deleted_label);
}

#include "graph_manager/hal_graph_widget.h"
#include "graph_relay/graph_relay.h"
#include "gui_globals.h"

#include <QVBoxLayout>
#include <QShortcut>
#include <QDebug>

hal_graph_widget::hal_graph_widget(QGraphicsView* view) : content_widget("graph"), m_graphics_widget(view)
{
    m_relayout_button = new QPushButton(this);
    m_relayout_button->setText("The netlist has been modified. Click here to re-layout the graph.");
    m_relayout_button->setHidden(true);

    m_content_layout->addWidget(m_relayout_button);
    m_content_layout->addWidget(&m_graphics_widget);

    m_selection_history_navigator = new selection_history_navigator(20, this);

    connect(m_relayout_button, &QPushButton::clicked, this, &hal_graph_widget::handle_relayout_button_clicked);
    connect(&g_graph_relay, &graph_relay::gate_event, this, &hal_graph_widget::handle_gate_event);
    connect(&g_graph_relay, &graph_relay::net_event, this, &hal_graph_widget::handle_net_event);

    QShortcut* back_shortcut = new QShortcut(QKeySequence(tr("a")), this);
    connect(back_shortcut, &QShortcut::activated, this, &hal_graph_widget::handle_back_activated);

    QShortcut* next_shortcut = new QShortcut(QKeySequence(tr("d")), this);
    connect(next_shortcut, &QShortcut::activated, this, &hal_graph_widget::handle_next_activated);

}

void hal_graph_widget::setup_toolbar(toolbar* toolbar)
{
    Q_UNUSED(toolbar)
}

void hal_graph_widget::handle_relayout_button_clicked()
{
    m_relayout_button->setHidden(true);
    g_selection_relay.relay_current_cleared(this);
    Q_EMIT relayout_button_clicked();
}


void hal_graph_widget::handle_back_activated()
{
    m_selection_history_navigator->navigate_to_prev_item();
}

void hal_graph_widget::handle_next_activated()
{
    m_selection_history_navigator->navigate_to_next_item();
}


void hal_graph_widget::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)
{
    Q_UNUSED(gate)
    Q_UNUSED(associated_data);

    if (ev == gate_event_handler::event::name_changed)
        return;
    else
        m_relayout_button->setHidden(false);
}

void hal_graph_widget::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)
{
    Q_UNUSED(net)
    Q_UNUSED(associated_data)

    if (ev == net_event_handler::event::name_changed)
        return;
    else
        m_relayout_button->setHidden(false);
}

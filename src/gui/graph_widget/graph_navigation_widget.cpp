#include "graph_widget/graph_navigation_widget.h"

#include "netlist/gate.h"

#include "gui_globals.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>

graph_navigation_widget::graph_navigation_widget(QWidget* parent) : QTableWidget(parent), m_from_gate(0), m_via_net(0)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    //setEditTriggers(QAbstractItemView::NoEditTriggers);

    setColumnCount(5);

    setHorizontalHeaderLabels({"ID", "Name", "Type", "Pin", "Submodule"});
    horizontalHeader()->setStretchLastSection(true);

    verticalHeader()->setVisible(false);

    //    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void graph_navigation_widget::setup()
{
    clearContents();

    switch (g_selection_relay.m_focus_type)
    {
        case selection_relay::item_type::none:
        {
            return;
        }
        case selection_relay::item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(g_selection_relay.m_focus_id);

            if (!g)
                return;

            m_from_gate = g_selection_relay.m_focus_id;

            std::string pin_type   = *std::next(g->get_output_pin_types().begin(), g_selection_relay.m_subfocus_index);
            std::shared_ptr<net> n = g->get_fan_out_net(pin_type);

            if (!n)
                return;

            fill_table(n);

            return;
        }
        case selection_relay::item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(g_selection_relay.m_focus_id);

            if (!n)
                return;

            std::shared_ptr<gate> g = n->get_src().get_gate();

            if (!g)
                return;

            m_from_gate = g->get_id();

            fill_table(n);

            return;
        }
        case selection_relay::item_type::module:
        {
            return;
        }
    }
}

void graph_navigation_widget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
        return QTableWidget::keyPressEvent(event);

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (selectedItems().isEmpty())
            return;

        std::shared_ptr<gate> g = g_netlist->get_gate_by_id(static_cast<const u32>(std::stoi(selectedItems().at(0)->text().toStdString())));

        if (!g)
            return;

        Q_EMIT navigation_requested(m_from_gate, m_via_net, g->get_id());
        return;
    }

    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        Q_EMIT close_requested();
}

void graph_navigation_widget::fill_table(std::shared_ptr<net> n)
{
    if (!n)
        return;

    m_via_net = n->get_id();

    setRowCount(n->get_dsts().size());

    int row = 0;

    for (const endpoint& e : n->get_dsts())
    {
        if (!e.gate)
            continue;

        QTableWidgetItem* item = new QTableWidgetItem(QString::number(e.get_gate()->get_id()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        setItem(row, 0, item);

        item = new QTableWidgetItem(QString::fromStdString(e.get_gate()->get_name()));
        item->setFlags(Qt::ItemIsSelectable);
        setItem(row, 1, item);

        item = new QTableWidgetItem(QString::fromStdString(e.get_gate()->get_type()->get_name()));
        item->setFlags(Qt::ItemIsSelectable);
        setItem(row, 2, item);

        item = new QTableWidgetItem(QString::fromStdString(e.get_pin_type()));
        item->setFlags(Qt::ItemIsSelectable);
        setItem(row, 3, item);

        // ADD SPECIAL SUBMODULE ITEM HERE
        item = new QTableWidgetItem("test");
        item->setFlags(Qt::ItemIsSelectable);
        setItem(row, 4, item);

        ++row;
    }

    selectRow(0);
    resizeRowsToContents();
    resizeColumnsToContents();

    int scrollbar_width = verticalScrollBar()->width();
    int total_width     = 0;

    for (int i = 0; i < horizontalHeader()->count(); ++i)
        total_width += horizontalHeader()->sectionSize(i);

    setFixedWidth(total_width + scrollbar_width);

    int scrollbar_height = horizontalScrollBar()->height();
    int header_height    = horizontalHeader()->height();

    int total_height = 0;

    for (int i = 0; i < verticalHeader()->count(); ++i)
        total_height += verticalHeader()->sectionSize(i);

    setFixedHeight(header_height + total_height + scrollbar_height);
}

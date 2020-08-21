#include "gui/graph_widget/graph_navigation_widget.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/module.h"

#include "gui/gui_globals.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>

#include <assert.h>

namespace hal
{
    GraphNavigationWidget::GraphNavigationWidget(QWidget* parent) : QTableWidget(parent), m_via_net(0)
    {
        m_hide_when_focus_lost = false;
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        //setEditTriggers(QAbstractItemView::NoEditTriggers);

        setColumnCount(5);

        setHorizontalHeaderLabels({"ID", "Name", "Type", "Pin", "Submodule"});
        horizontalHeader()->setStretchLastSection(true);

        verticalHeader()->setVisible(false);

        connect(this, &QTableWidget::itemDoubleClicked, this, &GraphNavigationWidget::handle_item_double_clicked);
    }

    void GraphNavigationWidget::setup(bool direction)
    {
        clearContents();

        switch (g_selection_relay->m_focus_type)
        {
            case SelectionRelay::item_type::none:
            {
                return;
            }
            case SelectionRelay::item_type::gate:
            {
                Gate* g = g_netlist->get_gate_by_id(g_selection_relay->m_focus_id);

                assert(g);

                m_origin = hal::node{hal::node_type::gate, g->get_id()};

                std::string pin_type   = (direction ? g->get_output_pins() : g->get_input_pins())[g_selection_relay->m_subfocus_index];
                Net* n = (direction ? g->get_fan_out_net(pin_type) : g->get_fan_in_net(pin_type));

                assert(n);

                fill_table(n, direction);

                return;
            }
            case SelectionRelay::item_type::net:
            {
                Net* n = g_netlist->get_net_by_id(g_selection_relay->m_focus_id);

                assert(n);
                assert(direction ? n->get_num_of_destinations() : n->get_num_of_sources());

                m_origin = hal::node{hal::node_type::gate, 0};

                fill_table(n, direction);

                return;
            }
            case SelectionRelay::item_type::module:
            {
                // TODO ???
                return;
            }
        }
    }

    void GraphNavigationWidget::setup(hal::node origin, Net* via_net, bool direction)
    {
        clearContents();
        fill_table(via_net, direction);
        m_origin = origin;
    }

    void GraphNavigationWidget::hide_when_focus_lost(bool hide)
    {
        m_hide_when_focus_lost = hide;
    }

    void GraphNavigationWidget::focusOutEvent(QFocusEvent* event)
    {
        Q_UNUSED(event);
        if (m_hide_when_focus_lost)
            hide();
    }

    void GraphNavigationWidget::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
        {
            return QTableWidget::keyPressEvent(event);
        }

        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Right)
        {
            commit_selection();
        }

        if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        {
            Q_EMIT close_requested();
            Q_EMIT reset_focus();
        }
    }

    void GraphNavigationWidget::fill_table(Net* n, bool direction)
    {
        assert(n);

        m_via_net = n->get_id();

        setRowCount(n->get_destinations().size() + 1);

        {
            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(0, 0, item);
            setItem(0, 2, item->clone());
            setItem(0, 3, item->clone());
            setItem(0, 4, item->clone());
            item = new QTableWidgetItem("Select All");
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(0, 1, item);

            // TODO are these items ever deleted or is this a memory leak?
        }

        int row = 1;

        for (const Endpoint& e : (direction ? n->get_destinations() : n->get_sources()))
        {
            if (!e.get_gate())
            {
                continue;
            }

            QTableWidgetItem* item = new QTableWidgetItem(QString::number(e.get_gate()->get_id()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 0, item);

            item = new QTableWidgetItem(QString::fromStdString(e.get_gate()->get_name()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 1, item);

            item = new QTableWidgetItem(QString::fromStdString(e.get_gate()->get_type()->get_name()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 2, item);

            item = new QTableWidgetItem(QString::fromStdString(e.get_pin()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 3, item);

            // ADD SPECIAL SUBMODULE ITEM HERE
            item = new QTableWidgetItem(QString::fromStdString(e.get_gate()->get_module()->get_name()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 4, item);

            ++row;
        }

        selectRow(0);
        resizeRowsToContents();
        resizeColumnsToContents();

        //    int scrollbar_width = verticalScrollBar()->width();
        //    int total_width = 0;

        //    for (int i = 0; i < horizontalHeader()->count(); ++i)
        //    {
        //        total_width += horizontalHeader()->sectionSize(i);
        //    }

        //    setFixedWidth(total_width + scrollbar_width);

        //    int scrollbar_height = horizontalScrollBar()->height();
        //    int header_height = horizontalHeader()->height();

        //    int total_height = 0;

        //    for (int i = 0; i < verticalHeader()->count(); ++i)
        //    {
        //        total_height += verticalHeader()->sectionSize(i);
        //    }

        //    setFixedHeight(header_height + total_height + scrollbar_height);

        //This version uses some magic numbers, but it does not behaves as wierd in
        //certain (random?) situations like the version above
        int width = verticalScrollBar()->width() + 40;
        for (int i = 0; i < columnCount(); i++)
            width += columnWidth(i);

        int height = horizontalHeader()->height() + 2;
        for (int i = 0; i < rowCount(); i++)
            height += rowHeight(i);

        int MAXIMUM_ALLOWED_HEIGHT = 500;
        setFixedWidth(width);
        setFixedHeight((height > MAXIMUM_ALLOWED_HEIGHT) ? MAXIMUM_ALLOWED_HEIGHT : height);
    }

    void GraphNavigationWidget::handle_item_double_clicked(QTableWidgetItem* item)
    {
        Q_UNUSED(item)
        commit_selection();
    }

    void GraphNavigationWidget::commit_selection()
    {
        if (selectedItems().isEmpty())
        {
            return;
        }

        if (selectedItems().at(0)->row() == 0)
        {
            // "select all" was chosen
            QSet<u32> gates;
            for (u32 row = 1; row < (u32)rowCount(); ++row)
            {
                Gate* g = g_netlist->get_gate_by_id(item(row, 0)->text().toLong());
                if (g)
                {
                    gates.insert(g->get_id());
                }
            }
            Q_EMIT navigation_requested(m_origin, m_via_net, gates, {});
            return;
        }

        Gate* g = g_netlist->get_gate_by_id(selectedItems().at(0)->text().toLong());

        if (!g)
        {
            return;
        }

        Q_EMIT navigation_requested(m_origin, m_via_net, {g->get_id()}, {});
        return;
    }
}

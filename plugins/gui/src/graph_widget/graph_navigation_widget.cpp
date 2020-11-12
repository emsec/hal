#include "gui/graph_widget/graph_navigation_widget.h"

#include "hal_core/utilities/log.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>

#include <assert.h>

namespace hal
{
    GraphNavigationWidget::GraphNavigationWidget(QWidget* parent) : QTableWidget(parent), mViaNet(0)
    {
        mHideWhenFocusLost = false;
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        //setEditTriggers(QAbstractItemView::NoEditTriggers);

        setColumnCount(5);

        setHorizontalHeaderLabels({"ID", "Name", "Type", "Pin", "Submodule"});
        horizontalHeader()->setStretchLastSection(true);

        verticalHeader()->setVisible(false);

        connect(this, &QTableWidget::itemDoubleClicked, this, &GraphNavigationWidget::handleItemDoubleClicked);
    }

    void GraphNavigationWidget::setup(bool direction)
    {
        clearContents();

        switch (gSelectionRelay->mFocusType)
        {
        case SelectionRelay::ItemType::None:
        {
            return;
        }
        case SelectionRelay::ItemType::Gate:
            {
                Gate* g = gNetlist->get_gate_by_id(gSelectionRelay->mFocusId);

                assert(g);

                mOrigin = Node(g->get_id(), Node::Gate);

                std::string pin_type   = (direction ? g->get_output_pins() : g->get_input_pins())[gSelectionRelay->mSubfocusIndex];
                Net* n = (direction ? g->get_fan_out_net(pin_type) : g->get_fan_in_net(pin_type));

                assert(n);

                fillTable(n, direction);

                return;
            }
        case SelectionRelay::ItemType::Net:
            {
                Net* n = gNetlist->get_net_by_id(gSelectionRelay->mFocusId);

                assert(n);
                assert(direction ? n->get_num_of_destinations() : n->get_num_of_sources());

                mOrigin = Node();

                fillTable(n, direction);

                return;
            }
        case SelectionRelay::ItemType::Module:
            {
                // TODO ???
                return;
            }
        }
    }

    void GraphNavigationWidget::setup(Node origin, Net* via_net, bool direction)
    {
        clearContents();
        fillTable(via_net, direction);
        mOrigin = origin;
    }

    void GraphNavigationWidget::hideWhenFocusLost(bool hide)
    {
        mHideWhenFocusLost = hide;
    }

    void GraphNavigationWidget::focusOutEvent(QFocusEvent* event)
    {
        Q_UNUSED(event);
        if (mHideWhenFocusLost)
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
            commitSelection();
        }

        if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        {
            Q_EMIT closeRequested();
            Q_EMIT resetFocus();
        }
    }

    void GraphNavigationWidget::fillTable(Net* n, bool direction)
    {
        assert(n);

        mViaNet = n->get_id();

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

        for (Endpoint* e : (direction ? n->get_destinations() : n->get_sources()))
        {
            if (!e->get_gate())
            {
                continue;
            }

            QTableWidgetItem* item = new QTableWidgetItem(QString::number(e->get_gate()->get_id()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 0, item);

            item = new QTableWidgetItem(QString::fromStdString(e->get_gate()->get_name()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 1, item);

            item = new QTableWidgetItem(QString::fromStdString(e->get_gate()->get_type()->get_name()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 2, item);

            item = new QTableWidgetItem(QString::fromStdString(e->get_pin()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            setItem(row, 3, item);

            // ADD SPECIAL SUBMODULE ITEM HERE
            item = new QTableWidgetItem(QString::fromStdString(e->get_gate()->get_module()->get_name()));
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

    void GraphNavigationWidget::handleItemDoubleClicked(QTableWidgetItem* item)
    {
        Q_UNUSED(item)
        commitSelection();
    }

    void GraphNavigationWidget::commitSelection()
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
                Gate* g = gNetlist->get_gate_by_id(item(row, 0)->text().toLong());
                if (g)
                {
                    gates.insert(g->get_id());
                }
            }
            Q_EMIT navigationRequested(mOrigin, mViaNet, gates, {});
            return;
        }

        Gate* g = gNetlist->get_gate_by_id(selectedItems().at(0)->text().toLong());

        if (!g)
        {
            return;
        }

        Q_EMIT navigationRequested(mOrigin, mViaNet, {g->get_id()}, {});
        return;
    }
}

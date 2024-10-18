#include "gui/selection_details_widget/net_details_widget/net_endpoint_table.h"

#include "gui/gui_globals.h"
#include "gui/module_context_menu/module_context_menu.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/selection_details_widget/net_details_widget/endpoint_table_model.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "hal_core/netlist/gate.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>

namespace hal
{

    NetEndpointTable::NetEndpointTable(EndpointTableModel* model, QWidget* parent) : QTableView(parent), mEndpointModel(model)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setFocusPolicy(Qt::NoFocus);
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
        setModel(mEndpointModel);

        setSelectionBehavior(QAbstractItemView::SelectRows);

        //connections
        connect(this, &QTableView::customContextMenuRequested, this, &NetEndpointTable::handleContextMenuRequested);
    }

    void NetEndpointTable::setNet(u32 netID)
    {
        Net* n = gNetlist->get_net_by_id(netID);

        setNet(n);
    }

    void NetEndpointTable::setNet(Net* n)
    {
        if (!n)
            return;

        mEndpointModel->setNet(n);

        QModelIndex unused;

        fitSizeToContent();

        Q_EMIT updateText(mEndpointModel->typeString() + " (" + QString::number(mEndpointModel->rowCount(unused)) + ")");
    }

    void NetEndpointTable::removeContent()
    {
        mEndpointModel->clear();
    }

    void NetEndpointTable::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = indexAt(pos);

        if (!idx.isValid())
            return;

        u32 gateID = mEndpointModel->getGateIDFromIndex(idx);
        Gate* gate = gNetlist->get_gate_by_id(gateID);
        QString pin = mEndpointModel->getPinNameFromIndex(idx);
        QString desc = mEndpointModel->typeString().toLower();
        QString netIDStr = QString::number(mEndpointModel->getCurrentNetID());
        QString gateIDStr = QString::number(gateID);

        QMenu menu;

        menu.addAction("Gate pin name to clipboard", [pin](){
            QApplication::clipboard()->setText(pin);
        });

        QString pythonCommand ="netlist.get_gate_by_id(" + gateIDStr + ").%1(\"" + pin + "\")";
        pythonCommand = (mEndpointModel->getType() == EndpointTableModel::Type::source) ? pythonCommand.arg("get_fan_out_endpoint") : pythonCommand.arg("get_fan_in_endpoint");

        menu.addAction(QIcon(":/icons/python"), "Extract endpoint as python code (copy to clipboard)", [pythonCommand](){
            QApplication::clipboard()->setText(pythonCommand);
        });

        ModuleContextMenu::addGateSubmenu(&menu, gateID);

        GuiPluginManager::addPluginSubmenus(&menu, gNetlist, 
            {}, std::vector<u32>({gateID}), {});

        menu.exec(this->viewport()->mapToGlobal(pos));
    }

    void NetEndpointTable::fitSizeToContent()
    {
        horizontalHeader()->setStretchLastSection(false);
        resizeRowsToContents();
        resizeColumnsToContents();

        int rows    = mEndpointModel->rowCount();
        int columns = mEndpointModel->columnCount();

        int w = 0;
        int h = 0;

        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the table's width far too big, so just set 0 as the size
        if (rows != 0)
        {
            w = verticalHeader()->width() + 4;    // +4 seems to be needed
            for (int i = 0; i < columns; i++)
                w += columnWidth(i);    // seems to include gridline

            h = horizontalHeader()->height() + 4;
            for (int i = 0; i < rows; i++)
                h += rowHeight(i);

            w = w + 5;    //no contemporary source exists why 5 is the magic number here (my guess would be it's the width of the hidden scrollbar)
        }
        setFixedHeight(h);
        setMinimumWidth(w);
        horizontalHeader()->setStretchLastSection(true);
        update();
        updateGeometry();
    }

    void NetEndpointTable::addSourceOurDestinationToSelection(u32 gateID, QString pin, bool clearSelection)
    {
        //setfocus action??
        auto gate = gNetlist->get_gate_by_id(gateID);
        if (!gate)
            return;
        if (clearSelection)
            gSelectionRelay->clear();
        gSelectionRelay->addGate(gateID);
        std::vector<std::string> pins;
        SelectionRelay::Subfocus focus;

        if (mEndpointModel->getType() == EndpointTableModel::Type::source)
        {
            pins  = gate->get_type()->get_output_pin_names();
            focus = SelectionRelay::Subfocus::Right;
        }
        else
        {
            pins  = gate->get_type()->get_input_pin_names();
            focus = SelectionRelay::Subfocus::Left;
        }
        auto index = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin.toStdString()));
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate, gateID, focus, index);
        gSelectionRelay->relaySelectionChanged(this);
    }
}    // namespace hal

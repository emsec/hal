#include "gui/new_selection_details_widget/new_net_details_widget/net_endpoint_table.h"
#include "gui/new_selection_details_widget/models/endpoint_table_model.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include "gui/new_selection_details_widget/py_code_provider.h"
#include <QDebug>

#include "gui/gui_globals.h"

namespace hal
{

    NetEndpointTable::NetEndpointTable(EndpointTableModel* model, QWidget* parent) : QTableView(parent), mEndpointModel(model)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

        if(!idx.isValid())
            return;

        u32 gateID = mEndpointModel->getGateIDFromIndex(idx);

        QMenu menu;

        //menu.addSection("Python");

        QString pythonCommandGate = PyCodeProvider::pyCodeGate(gateID);
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code",
            [pythonCommandGate]()
            {
                QApplication::clipboard()->setText(pythonCommandGate);
            });

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void NetEndpointTable::fitSizeToContent()
    {
        resizeRowsToContents();
        int h = horizontalHeader()->height() + 4;
        for (int i = 0; i < mEndpointModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);
    }
}

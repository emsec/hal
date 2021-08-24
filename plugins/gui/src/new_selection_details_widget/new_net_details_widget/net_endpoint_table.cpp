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

    NetEndpointTable::NetEndpointTable(EndpointTableModel* model, QWidget* parent) : QTableView(parent), mSourceModel(model)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
        setModel(mSourceModel);

        setSelectionBehavior(QAbstractItemView::SelectRows);

        //connections
        connect(this, &QTableView::customContextMenuRequested, this, &NetEndpointTable::handleContextMenuRequested);


        // DEBUG
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &NetEndpointTable::debugHandleShortcut);

    }

    void NetEndpointTable::setContent(u32 netID)
    {
        Net* n = gNetlist->get_net_by_id(netID);

        setContent(n);
    }

    void NetEndpointTable::setContent(Net* n)
    {
        if (!n)
            return;

        mSourceModel->setNet(n);
    }

    void NetEndpointTable::removeContent()
    {
        mSourceModel->clear();
    }

    void NetEndpointTable::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = indexAt(pos);

        if(!idx.isValid())
            return;

        u32 gateID = mSourceModel->getGateIDFromIndex(idx);

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

    void NetEndpointTable::debugHandleShortcut(void* sender)
    {
        qDebug() << "called correctly";
        static int counter = 1;

        Q_EMIT update_text("Destinations (" + QString::number(counter) + ")");

        ++counter;
    }

}

#include "gui/new_selection_details_widget/new_net_details_widget/net_module_table.h"

#include "hal_core/netlist/gate.h"

#include "gui/new_selection_details_widget/models/module_table_model.h"
#include "gui/gui_globals.h"
#include "gui/new_selection_details_widget/py_code_provider.h"

#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMenu>

namespace hal
{

    NetModuleTable::NetModuleTable(ModuleTableModel* model, QWidget* parent) : QTableView(parent), mSourceModel(model)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
        setModel(mSourceModel);

        setSelectionBehavior(QAbstractItemView::SelectRows);

        //connections
        connect(this, &QTableView::customContextMenuRequested, this, &NetModuleTable::handleContextMenuRequested);

    }

    void NetModuleTable::setContent(u32 netID)
    {
        Net* n = gNetlist->get_net_by_id(netID);

        setContent(n);
    }

    void NetModuleTable::setContent(Net* n)
    {
        if (!n)
            return;

        mSourceModel->setNet(n);
    }

    void NetModuleTable::removeContent()
    {
        mSourceModel->clear();
    }

    void NetModuleTable::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = indexAt(pos);

        if(!idx.isValid())
            return;

        u32 gateID = mSourceModel->getModuleIDFromIndex(idx);

        QMenu menu;

        //menu.addSection("Python");

        QString pythonCommandGate = PyCodeProvider::pyCodeGate(gateID);
        menu.addAction(QIcon(":/icons/python"), "Extract module as python code",
            [pythonCommandGate]()
            {
                QApplication::clipboard()->setText(pythonCommandGate);
            });

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

}

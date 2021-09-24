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

    NetModuleTable::NetModuleTable(ModuleTableModel* model, QWidget* parent) : QTableView(parent), mModuleTableModel(model)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
        setModel(mModuleTableModel);

        setSelectionBehavior(QAbstractItemView::SelectRows);

        //connections
        connect(this, &QTableView::customContextMenuRequested, this, &NetModuleTable::handleContextMenuRequested);

    }

    void NetModuleTable::setNet(u32 netID)
    {
        Net* n = gNetlist->get_net_by_id(netID);

        setNet(n);
    }

    void NetModuleTable::setNet(Net* n)
    {
        if (!n)
            return;

        mModuleTableModel->setNet(n);

        QModelIndex unused;

        fitSizeToContent();

        Q_EMIT updateText("Modules (" + QString::number(mModuleTableModel->rowCount(unused)) + ")");
    }

    void NetModuleTable::removeContent()
    {
        mModuleTableModel->clear();
    }

    void NetModuleTable::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = indexAt(pos);

        if(!idx.isValid())
            return;

        u32 moduleID = mModuleTableModel->getModuleIDFromIndex(idx);

        QMenu menu;

        QString portname = mModuleTableModel->getPortNameFromIndex(idx);
        menu.addAction(QIcon(), "Extract port name as plain text",
            [portname]()
            {
                QApplication::clipboard()->setText(portname);
            });

        //menu.addSection("Python");

        QString pythonCommandModule = PyCodeProvider::pyCodeModule(moduleID);
        menu.addAction(QIcon(":/icons/python"), "Extract module as python code",
            [pythonCommandModule]()
            {
                QApplication::clipboard()->setText(pythonCommandModule);
            });

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void NetModuleTable::fitSizeToContent()
    {
        resizeRowsToContents();
        int h = horizontalHeader()->height() + 4;
        for (int i = 0; i < mModuleTableModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);
    }

}

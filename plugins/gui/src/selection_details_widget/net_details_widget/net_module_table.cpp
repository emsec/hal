#include "gui/selection_details_widget/net_details_widget/net_module_table.h"

#include "hal_core/netlist/gate.h"

#include "gui/selection_details_widget/net_details_widget/module_table_model.h"
#include "gui/gui_globals.h"
#include "gui/module_context_menu/module_context_menu.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/python/py_code_provider.h"

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
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);
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
        u32 pinID = mModuleTableModel->getPinIDFromIndex(idx);
        QString pinName = mModuleTableModel->getPortNameFromIndex(idx);

        QMenu menu;

        menu.addAction("Pin name to clipboard", [pinName](){
            QApplication::clipboard()->setText(pinName);
        });
        menu.addAction(QIcon(":/icons/python"), "Extract endpoint as python code (copy to clipboard)", 
            [moduleID, pinID]()
            {QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinById(moduleID, pinID));}
        );

        ModuleContextMenu::addModuleSubmenu(&menu, moduleID);

        GuiPluginManager::addPluginSubmenus(&menu, gNetlist, 
            std::vector<u32>({moduleID}), {}, {});

        menu.exec(this->viewport()->mapToGlobal(pos));
    }

    void NetModuleTable::fitSizeToContent()
    {
        horizontalHeader()->setStretchLastSection(false);
        resizeRowsToContents();
        resizeColumnsToContents();

        int rows = mModuleTableModel->rowCount();
        int columns = mModuleTableModel->columnCount();

        int w = 0;
        int h = 0;

        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the table's width far too big, so just set 0 as the size
        if(rows != 0)
        {
            w = verticalHeader()->width() + 4;    // +4 seems to be needed
            for (int i = 0; i < columns; i++)
                w += columnWidth(i);    // seems to include gridline

            h = horizontalHeader()->height() + 4;
            for (int i = 0; i < rows; i++)
                h += rowHeight(i);

            w = w + 5; //no contemporary source exists why 5 is the magic number here (my guess would be it's the width of the hidden scrollbar)
        }
        setFixedHeight(h);
        setMinimumWidth(w);
        horizontalHeader()->setStretchLastSection(true);
        update();
        updateGeometry();
    }

}

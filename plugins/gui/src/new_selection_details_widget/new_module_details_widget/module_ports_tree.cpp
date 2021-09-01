#include "gui/new_selection_details_widget/new_module_details_widget/module_ports_tree.h"
#include "gui/new_selection_details_widget/models/port_tree_model.h"
#include <QHeaderView>
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{

    ModulePortsTree::ModulePortsTree(QWidget *parent) : QTreeView(parent), mPortModel(new PortTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        header()->setStretchLastSection(true);
        setModel(mPortModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModulePortsTree::handleContextMenuRequested);
        connect(mPortModel, &PortTreeModel::numberOfPortsChanged, this, &ModulePortsTree::handleNumberOfPortsChanged);

    }

    void ModulePortsTree::setModule(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        mPortModel->setModule(m);
        mModuleID = moduleID;
        //adjustSize();
    }

    void ModulePortsTree::setModule(Module *m)
    {
        if(!m) return;

        mPortModel->setModule(m);
        adjustSize();
        qDebug() << "I am called!!!!!!!!!";
    }

    void ModulePortsTree::removeContent()
    {
        mPortModel->clear();
        mModuleID = -1;
    }

    void ModulePortsTree::handleContextMenuRequested(const QPoint &pos)
    {
        Q_UNUSED(pos)
    }

    void ModulePortsTree::adjustSize()
    {
        header()->setStretchLastSection(false); //hacky shit
        for(int i = 0; i < mPortModel->columnCount(); i++)
            resizeColumnToContents(i);


        int rows = mPortModel->rowCount();
        int columns = mPortModel->columnCount();

        int w = 0;
        int h = 0;

        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the table's width far too big, so just set 0 as the size

        if(rows != 0)
        {
            //w = header()->width() + 4;    // +4 seems to be needed

            for (int i = 0; i < columns; i++){
                //qDebug() << "WIDTH FOR COLUMN " << i << ": " << columnWidth(i);
                w += columnWidth(i);    // seems to include gridline
            }
            //height depends on which items are collapsed and is therefore "dynamic"->stretch the height
            //h = horizontalHeader()->height() + 4
//            for (int i = 0; i < rows; i++)
//                h += rowHeight(QModelIndex());

            //w = w + 5; //no contemporary source exists why 5 is the magic number here (my guess would be it's the width of the hidden scrollbar)
        }

        //qDebug() << "WIDTH: " << w;
        setMinimumSize(QSize(w, 2000));
        header()->setStretchLastSection(true); //hacky shit 2
    }


    void ModulePortsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        //adjustSize();
        Q_EMIT updateText(QString("Ports (%1)").arg(newNumberPorts));
    }


}

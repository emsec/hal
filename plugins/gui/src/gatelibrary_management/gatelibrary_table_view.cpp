#include "gui/gatelibrary_management/gatelibrary_table_view.h"

#include "gui/gui_globals.h"

#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMenu>

namespace hal
{

    GatelibraryTableView::GatelibraryTableView(QWidget* parent) : QTableView(parent)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();

        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);

        //connections
        connect(this, &QTableView::customContextMenuRequested, this, &GatelibraryTableView::handleContextMenuRequested);

    }

    void GatelibraryTableView::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = indexAt(pos);

        if(!idx.isValid())
            return;

        QMenu menu;

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

}

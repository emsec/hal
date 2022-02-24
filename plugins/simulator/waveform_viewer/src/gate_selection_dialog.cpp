#include "waveform_viewer/gate_selection_dialog.h"

#include "gui/module_dialog/gate_select_model.h"
#include <QGridLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTableView>
#include <QHeaderView>

#include "gui/gui_globals.h"

namespace hal {


    GateSelectionDialog::GateSelectionDialog(QWidget *parent)
        : QDialog(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        mButAll = new QPushButton("All gates", this);
        connect(mButAll,&QPushButton::clicked,this,&GateSelectionDialog::handleSelectAll);
        layout->addWidget(mButAll,0,0);
        mButSel = new QPushButton("Current GUI selection", this);
        connect(mButSel,&QPushButton::clicked,this,&GateSelectionDialog::handleCurrentGuiSelection);
        layout->addWidget(mButSel,0,1);
        mButNone = new QPushButton("Clear selection", this);
        connect(mButNone,&QPushButton::clicked,this,&GateSelectionDialog::handleClearSelection);
        layout->addWidget(mButNone,0,2);
        mTableView = new QTableView(this);

        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::MultiSelection);

        GateSelectProxy* prox = new GateSelectProxy(this);
        // connect(sbar, &Searchbar::textEdited, prox, &GateSelectProxy::searchTextChanged);

        GateSelectModel* modl = new GateSelectModel(false,QSet<u32>(),mTableView);
        prox->setSourceModel(modl);
        mTableView->setModel(prox);

        // connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &GateSelectView::handleCurrentChanged);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(2, Qt::AscendingOrder);
        mTableView->resizeColumnsToContents();
        mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->verticalHeader()->hide();
        layout->addWidget(mTableView,1,0,1,3);
        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(mButtonBox,2,1,1,2);
    }

    void GateSelectionDialog::handleSelectAll()
    {
        mTableView->selectAll();
    }

    void GateSelectionDialog::handleCurrentGuiSelection()
    {
        QSet<u32> guiGateSel = gSelectionRelay->selectedGates();

        const QAbstractItemModel* modl = mTableView->model(); // proxy model
        int nrows = modl->rowCount();
        mTableView->clearSelection();

        bool ok;

        for (int irow = 0; irow<nrows; irow++)
        {
            u32 gid = modl->data(modl->index(irow,0)).toUInt(&ok);
            if (!ok) continue;
            if (guiGateSel.contains(gid))
                mTableView->selectRow(irow);
        }
    }

    void GateSelectionDialog::handleClearSelection()
    {
        mTableView->clearSelection();
    }

    std::vector<Gate*> GateSelectionDialog::selectedGates() const
    {
        std::vector<Gate*> retval;
        QItemSelectionModel *sm = mTableView->selectionModel();
        if (!sm->hasSelection()) return retval;
        QSet<u32> selGates;
        bool ok;
        for (const QModelIndex& inx : sm->selectedRows(0))
        {
            u32 gid = mTableView->model()->data(inx).toUInt(&ok);
            if (!ok) continue;
            selGates.insert(gid);
        }

        for (u32 gid: selGates)
        {
            Gate* g = gNetlist->get_gate_by_id(gid);
            if (g) retval.push_back(g);
        }
        return retval;
    }
}

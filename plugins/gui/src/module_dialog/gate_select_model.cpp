#include "gui/module_dialog/gate_select_model.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QSortFilterProxyModel>

namespace hal
{
    //---------------- HISTORY ----------------------------------------
    GateSelectHistory* GateSelectHistory::inst = nullptr;

    GateSelectHistory* GateSelectHistory::instance()
    {
        if (!inst)
            inst = new GateSelectHistory;
        return inst;
    }

    void GateSelectHistory::add(u32 id)
    {
        removeAll(id);
        prepend(id);
    }

    //---------------- ENTRY ------------------------------------------
    GateSelectEntry::GateSelectEntry(Gate* g)
    {
        mId    = g->get_id();
        mName  = QString::fromStdString(g->get_name());
        mType  = QString::fromStdString(g->get_type()->get_name());
    }

    QVariant GateSelectEntry::data(int icol) const
    {
        switch (icol)
        {
            case 0:
                return mId;
            case 1:
                return mName;
            case 2:
                return mType;
        }
        return QVariant();
    }

    //---------------- MODEL ------------------------------------------
    GateSelectModel::GateSelectModel(bool history, const QSet<u32> &selectable, QObject* parent)
        : QAbstractTableModel(parent), mSelectableGates(selectable)
    {
        if (history)
        {
            for (u32 id : *GateSelectHistory::instance())
            {
                Gate* g = gNetlist->get_gate_by_id(id);
                if (g && isAccepted(g->get_id(),mSelectableGates))
                    mEntries.append(GateSelectEntry(g));
            }
        }
        else
        {
            for (Gate* g : gNetlist->get_gates())
                if (isAccepted(g->get_id(),mSelectableGates))
                    mEntries.append(GateSelectEntry(g));
        }
    }

    bool GateSelectModel::isAccepted(u32 gateId, const QSet<u32> &selectable)
    {
        if (selectable.isEmpty()) return true;
        return selectable.contains(gateId);
    }

    int GateSelectModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);

        return mEntries.size();
    }

    int GateSelectModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);

        return 3;
    }

    QVariant GateSelectModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();
        switch (section)
        {
            case 0:
                return "ID";
            case 1:
                return "Name";
            case 2:
                return "Type";
        }
        return QVariant();
    }

    QVariant GateSelectModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() >= mEntries.size())
            return QVariant();
        const GateSelectEntry& mod = mEntries.at(index.row());

        if (role == Qt::DisplayRole)
           return mod.data(index.column());

        return QVariant();
    }

    u32 GateSelectModel::gateId(int irow) const
    {
        return mEntries.at(irow).id();
    }

    //---------------- PROXY ------------------------------------------
    GateSelectProxy::GateSelectProxy(QObject* parent) : QSortFilterProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::numerated)
    {
        ;
    }

    void GateSelectProxy::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
    }

    bool GateSelectProxy::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
    {
        if (!sourceLeft.column() && !sourceRight.column())
            return sourceModel()->data(sourceLeft).toInt() < sourceModel()->data(sourceRight).toInt();

        QString sLeft  = sourceModel()->data(sourceLeft).toString();
        QString sRight = sourceModel()->data(sourceRight).toString();
        return gui_utility::compare(mSortMechanism, sLeft, sRight);
    }

    void GateSelectProxy::searchTextChanged(const QString& txt)
    {
        setFilterKeyColumn(-1);
        setFilterRegularExpression(txt);
    }

    //---------------- PICKER -----------------------------------------
    GateSelectPicker* GateSelectPicker::sCurrentPicker = nullptr;

    GateSelectPicker::GateSelectPicker(u32 orig, bool succ, const QSet<u32> &selectable)
        : mOrigin(orig), mPickSuccessor(succ), mSelectableGates(selectable)
    {
        if (sCurrentPicker)
            sCurrentPicker->deleteLater();
        connect(this, &GateSelectPicker::triggerCursor, gContentManager->getGraphTabWidget(), &GraphTabWidget::setSelectCursor);
        sCurrentPicker = this;
        Q_EMIT(triggerCursor(GraphTabWidget::PickGate));
    }

    void GateSelectPicker::handleSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);

        Gate* firstAccepted = nullptr;
        u32 notAccepted       = 0;
        bool terminate        = true;

        for (u32 gatId : gSelectionRelay->selectedGatesList())
        {
            if (GateSelectModel::isAccepted(gatId,mSelectableGates))
            {
                Gate* g = gNetlist->get_gate_by_id(gatId);
                if (g)
                {
                    firstAccepted = g;
                    break;
                }
            }
            else
            {
                if (!notAccepted)
                    notAccepted = gatId;
            }
        }

        if (firstAccepted)
        {
            if (mPickSuccessor)
                Q_EMIT(gatesPicked(mOrigin,firstAccepted->get_id()));
            else
                Q_EMIT(gatesPicked(firstAccepted->get_id(),mOrigin));
        }
        else if (notAccepted)
        {
            Gate* g = gNetlist->get_gate_by_id(notAccepted);
            if (g)
                QMessageBox::warning(qApp->activeWindow(), "Warning", QString("Gate %1[%2] is no %3")
                                     .arg(QString::fromStdString(g->get_name()))
                                     .arg(notAccepted)
                                     .arg(mPickSuccessor ? "successor" : "predecessor"));
            else
                QMessageBox::warning(qApp->activeWindow(), "Warning", QString("Cannot select gate [%1]").arg(notAccepted));
        }
        else
            terminate = false;

        if (terminate)
            terminateCurrentPicker();
    }

    void GateSelectPicker::terminateCurrentPicker()
    {
        if (!sCurrentPicker)
            return;
        GateSelectPicker* toDelete = sCurrentPicker;
        sCurrentPicker               = nullptr;
        toDelete->triggerCursor(false);
        disconnect(gSelectionRelay, &SelectionRelay::selectionChanged, toDelete, &GateSelectPicker::handleSelectionChanged);
        toDelete->deleteLater();
    }

    //---------------- VIEW -------------------------------------------
    GateSelectView::GateSelectView(bool history, Searchbar* sbar, const QSet<u32> &selectable, QWidget* parent) : QTableView(parent)
    {
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);

        GateSelectProxy* prox = new GateSelectProxy(this);
        connect(sbar, &Searchbar::textEdited, prox, &GateSelectProxy::searchTextChanged);

        GateSelectModel* modl = new GateSelectModel(history, selectable, this);
        prox->setSourceModel(modl);
        setModel(prox);

        connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &GateSelectView::handleCurrentChanged);
        connect(this, &QTableView::doubleClicked, this, &GateSelectView::handleDoubleClick);
        setSortingEnabled(true);
        sortByColumn(history ? -1 : 2, Qt::AscendingOrder);
        resizeColumnsToContents();
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
    }

    void GateSelectView::handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);
        const GateSelectProxy* prox = static_cast<const GateSelectProxy*>(model());
        Q_ASSERT(prox);
        const GateSelectModel* modl = static_cast<const GateSelectModel*>(prox->sourceModel());
        QModelIndex sourceIndex       = prox->mapToSource(current);
        u32 selGatId                 = modl->gateId(sourceIndex.row());
        Q_EMIT(gateSelected(selGatId, false));
    }

    void GateSelectView::handleDoubleClick(const QModelIndex& index)
    {
        const GateSelectProxy* prox = static_cast<const GateSelectProxy*>(model());
        Q_ASSERT(prox);
        const GateSelectModel* modl = static_cast<const GateSelectModel*>(prox->sourceModel());
        QModelIndex sourceIndex       = prox->mapToSource(index);
        u32 selGatId                  = modl->gateId(sourceIndex.row());
        Q_EMIT(gateSelected(selGatId, true));
    }

}    // namespace hal

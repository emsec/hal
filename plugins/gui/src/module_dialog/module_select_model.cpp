#include "gui/module_dialog/module_select_model.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/hal_qt_compat/hal_qt_compat.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "hal_core/netlist/module.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <QDebug>

namespace hal
{
    //---------------- HISTORY ----------------------------------------
    const int ModuleSelectHistory::sMaxEntries = 10;

    ModuleSelectHistory* ModuleSelectHistory::inst = nullptr;

    ModuleSelectHistory* ModuleSelectHistory::instance()
    {
        if (!inst)
            inst = new ModuleSelectHistory;
        return inst;
    }

    void ModuleSelectHistory::add(u32 id)
    {
        removeAll(id);
        prepend(id);
        while (size() > sMaxEntries) takeLast();
    }

    //---------------- ENTRY ------------------------------------------
    ModuleSelectEntry::ModuleSelectEntry(Module* m)
    {
        mId    = m->get_id();
        mName  = QString::fromStdString(m->get_name());
        mType  = QString::fromStdString(m->get_type());
        mColor = gNetlistRelay->getModuleColor(mId);
    }

    QVariant ModuleSelectEntry::data(int icol) const
    {
        switch (icol)
        {
            case 0:
                return mColor;
            case 1:
                return mId;
            case 2:
                return mName;
            case 3:
                return mType;
        }
        return QVariant();
    }

    //---------------- MODEL ------------------------------------------
    ModuleSelectModel::ModuleSelectModel(QObject* parent) : QAbstractTableModel(parent)
    {

    }

    void ModuleSelectModel::appendEntries(bool history)
    {
        if (history)
        {
            for (u32 id : *ModuleSelectHistory::instance())
            {
                Module* m = gNetlist->get_module_by_id(id);
                if (m && mExcl.isAccepted(m->get_id()))
                    mEntries.append(ModuleSelectEntry(m));
            }
        }
        else
        {
            for (Module* m : gNetlist->get_modules())
            {
                if (mExcl.isAccepted(m->get_id()))
                {
                    mEntries.append(ModuleSelectEntry(m));
                }
            }
        }
    }

    int ModuleSelectModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);

        return mEntries.size();
    }

    int ModuleSelectModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);

        return 4;
    }

    QVariant ModuleSelectModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();
        switch (section)
        {
            case 1:
                return "ID";
            case 2:
                return "Name";
            case 3:
                return "Type";
        }
        return QVariant();
    }

    QVariant ModuleSelectModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() >= mEntries.size())
            return QVariant();
        const ModuleSelectEntry& mod = mEntries.at(index.row());

        switch (role)
        {
            case Qt::DecorationRole: {
                if (index.column() == 0)
                {
                    QString runIconStyle = "all->" + mod.color().name();
                    QString runIconPath  = ":/icons/filled-circle";

                    return gui_utility::getStyledSvgIcon(runIconStyle, runIconPath);
                }
                break;
            }
            case Qt::DisplayRole:
                if (index.column() > 0)
                    return mod.data(index.column());
                break;
        }

        return QVariant();
    }

    u32 ModuleSelectModel::moduleId(int irow) const
    {
        return mEntries.at(irow).id();
    }

    QColor ModuleSelectModel::moduleColor(int irow) const
    {
        return mEntries.at(irow).color();
    }

    //---------------- PROXY ------------------------------------------
    ModuleSelectProxy::ModuleSelectProxy(QObject* parent) : SearchProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::numerated)
    {
        ;
    }

    void ModuleSelectProxy::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
    }

    bool ModuleSelectProxy::lessThan(const QColor& a, const QColor& b)
    {
        if (a.hue() < b.hue())
            return true;
        if (a.hue() > b.hue())
            return false;
        if (a.saturation() < b.saturation())
            return true;
        if (a.saturation() > b.saturation())
            return false;
        return (a.value() < b.value());
    }

    bool ModuleSelectProxy::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
    {
        if (!sourceLeft.column() && !sourceRight.column())
        {
            const ModuleSelectModel* modl = static_cast<const ModuleSelectModel*>(sourceModel());
            QColor cLeft                  = modl->moduleColor(sourceLeft.row());
            QColor cRight                 = modl->moduleColor(sourceRight.row());
            return lessThan(cLeft, cRight);
        }
        QString sLeft  = sourceModel()->data(sourceLeft).toString();
        QString sRight = sourceModel()->data(sourceRight).toString();
        return gui_utility::compare(mSortMechanism, sLeft, sRight);
    }

    void ModuleSelectProxy::startSearch(QString text, int options)
    {      
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }

    bool ModuleSelectProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        return checkRow(source_row, source_parent, 1, 3, 1);
    }

    //---------------- EXCLUDE ----------------------------------------
    ModuleSelectExclude::ModuleSelectExclude()
    {
        mModules = gSelectionRelay->selectedModulesList();
        mGates   = gSelectionRelay->selectedGatesList();
    }

    QString ModuleSelectExclude::selectionToString() const
    {
        QString retval;
        if (!mModules.isEmpty())
        {
            retval += "module";
            if (mModules.size() > 1)
                retval += 's';
            for (u32 modId : mModules)
            {
                Module* m = gNetlist->get_module_by_id(modId);
                if (m)
                    retval += QString(" '%1'[%2]").arg(m->get_name().c_str()).arg(m->get_id());
            }
        }
        if (!retval.isEmpty())
            retval += ", ";
        if (!mGates.isEmpty())
        {
            retval += "gate";
            if (mGates.size() > 1)
                retval += 's';
            for (u32 modId : mGates)
            {
                Gate* g = gNetlist->get_gate_by_id(modId);
                if (g)
                    retval += QString(" '%1'[%2]").arg(g->get_name().c_str()).arg(g->get_id());
            }
        }
        return retval;
    } 

    QSet<u32> ModuleSelectExclude::modules() const
    {
        return QtCompat::listToSet<u32>(mModules);
    }

    QSet<u32> ModuleSelectExclude::gates() const
    {
        return QtCompat::listToSet<u32>(mGates);
    }

    //---------------- PICKER -----------------------------------------

    ModuleSelectPicker::ModuleSelectPicker(ModuleSelectReceiver* receiver, QObject* parent)
        : QObject(parent)
    {
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &ModuleSelectPicker::handleSelectionChanged);
        connect(gContentManager->getGraphTabWidget(),&GraphTabWidget::triggerTerminatePicker,this,&ModuleSelectPicker::terminatePicker);
        connect(this, &ModuleSelectPicker::triggerCursor, gContentManager->getGraphTabWidget(), &GraphTabWidget::setSelectCursor);
        connect(this, &ModuleSelectPicker::modulesPicked, receiver, &ModuleSelectReceiver::handleModulesPicked);
        Q_EMIT triggerCursor(GraphTabWidget::PickModule);
    }

    void ModuleSelectPicker::handleSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);

        Module* firstAccepted = nullptr;
        u32 notAccepted       = 0;
        bool terminate        = true;

        for (u32 modId : gSelectionRelay->selectedModulesList())
        {
            if (mSelectExclude.isAccepted(modId))
            {
                Module* m = gNetlist->get_module_by_id(modId);
                if (m)
                {
                    firstAccepted = m;
                    break;
                }
            }
            else
            {
                if (!notAccepted)
                    notAccepted = modId;
            }
        }

        if (firstAccepted)
        {
            mModulesSelected.insert(firstAccepted->get_id());
        }
        else if (notAccepted)
        {
            Module* mRefused = gNetlist->get_module_by_id(notAccepted);
            if (mRefused)
                QMessageBox::warning(qApp->activeWindow(), "Warning", QString("Cannot select module '%1' [id=%2]").arg(QString::fromStdString(mRefused->get_name())).arg(notAccepted));
            else
                QMessageBox::warning(qApp->activeWindow(), "Warning", QString("Module with id=%1 not found in netlist").arg(notAccepted));
        }
        else
            terminate = gSelectionRelay->numberSelectedItems() > 0;

        if (terminate)
            terminatePicker();
    }

    void ModuleSelectPicker::terminatePicker()
    {
        disconnect(gContentManager->getGraphTabWidget(),&GraphTabWidget::triggerTerminatePicker,this,&ModuleSelectPicker::terminatePicker);
        disconnect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &ModuleSelectPicker::handleSelectionChanged);
        Q_EMIT modulesPicked(mModulesSelected);
        Q_EMIT triggerCursor(GraphTabWidget::Select);
        deleteLater();
    }

    //---------------- VIEW -------------------------------------------
    ModuleSelectView::ModuleSelectView(bool history, Searchbar* sbar, QSet<u32>* exclude_ids, QWidget* parent) : QTableView(parent)
    {
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);

        ModuleSelectProxy* prox = new ModuleSelectProxy(this);
        connect(sbar, &Searchbar::triggerNewSearch, prox, &ModuleSelectProxy::startSearch);

        ModuleSelectModel* modl = new ModuleSelectModel(this);

        if (exclude_ids != nullptr)
        {
            modl->excludeModulesById(*exclude_ids);
        }
        modl->appendEntries(history);

        prox->setSourceModel(modl);
        setModel(prox);

        connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ModuleSelectView::handleSelectionChanged);
        connect(this, &QTableView::doubleClicked, this, &ModuleSelectView::handleDoubleClick);
        setSortingEnabled(true);
        sortByColumn(history ? -1 : 2, Qt::AscendingOrder);
        resizeColumnsToContents();
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
    }

    void ModuleSelectView::handleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
        if (selected.indexes().empty())
        {
            Q_EMIT(moduleSelected(0, false));
            return;
        }
        Q_UNUSED(deselected);
        const ModuleSelectProxy* prox = static_cast<const ModuleSelectProxy*>(model());
        Q_ASSERT(prox);
        const ModuleSelectModel* modl = static_cast<const ModuleSelectModel*>(prox->sourceModel());
        QModelIndex sourceIndex       = prox->mapToSource(selected.indexes().at(0));
        u32 selModId                  = modl->moduleId(sourceIndex.row());
        Q_EMIT(moduleSelected(selModId, false));
    }

    void ModuleSelectView::handleDoubleClick(const QModelIndex& index)
    {
        const ModuleSelectProxy* prox = static_cast<const ModuleSelectProxy*>(model());
        Q_ASSERT(prox);
        const ModuleSelectModel* modl = static_cast<const ModuleSelectModel*>(prox->sourceModel());
        QModelIndex sourceIndex       = prox->mapToSource(index);
        u32 selModId                  = modl->moduleId(sourceIndex.row());
        Q_EMIT(moduleSelected(selModId, true));
    }

}    // namespace hal

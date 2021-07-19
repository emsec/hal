#include "gui/module_dialog/module_select_model.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/module_dialog/module_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "hal_core/netlist/module.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QSortFilterProxyModel>

namespace hal
{
    //---------------- HISTORY ----------------------------------------
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
    ModuleSelectModel::ModuleSelectModel(bool history, QObject* parent) : QAbstractTableModel(parent)
    {
        ModuleSelectExclude excl;
        if (history)
        {
            for (u32 id : *ModuleSelectHistory::instance())
            {
                Module* m = gNetlist->get_module_by_id(id);
                if (m && excl.isAccepted(m->get_id()))
                    mEntries.append(ModuleSelectEntry(m));
            }
        }
        else
        {
            for (Module* m : gNetlist->get_modules())
                if (excl.isAccepted(m->get_id()))
                    mEntries.append(ModuleSelectEntry(m));
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
    ModuleSelectProxy::ModuleSelectProxy(QObject* parent) : QSortFilterProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::numerated)
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

    void ModuleSelectProxy::searchTextChanged(const QString& txt)
    {
        setFilterKeyColumn(-1);
        setFilterRegularExpression(txt);
    }

    //---------------- EXCLUDE ----------------------------------------
    ModuleSelectExclude::ModuleSelectExclude()
    {
        mModules = gSelectionRelay->selectedModulesList();
        mGates   = gSelectionRelay->selectedGatesList();

        for (u32 gid : mGates)
        {
            Gate* g = gNetlist->get_gate_by_id(gid);
            if (!g)
                continue;
            mExclude.insert(g->get_module()->get_id());
        }

        for (u32 mid : mModules)
        {
            mExclude.insert(mid);
            Module* m = gNetlist->get_module_by_id(mid);
            if (!m)
                continue;
            Module* pm = m->get_parent_module();
            if (pm)
                mExclude.insert(pm->get_id());
            for (Module* sm : m->get_submodules(nullptr, true))
                mExclude.insert(sm->get_id());
        }
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

    //---------------- PICKER -----------------------------------------
    ModuleSelectPicker* ModuleSelectPicker::sCurrentPicker = nullptr;

    ModuleSelectPicker::ModuleSelectPicker()
    {
        if (sCurrentPicker)
            sCurrentPicker->deleteLater();
        connect(this, &ModuleSelectPicker::triggerCursor, gContentManager->getGraphTabWidget(), &GraphTabWidget::setSelectCursor);
        sCurrentPicker = this;
        Q_EMIT(triggerCursor(GraphTabWidget::PickModule));
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
            u32 moduleId = firstAccepted->get_id();
            if (QMessageBox::question(qApp->activeWindow(),
                                      "Confirm:",
                                      QString("Ok to move %1 into module '%2'[%3]").arg(mSelectExclude.selectionToString()).arg(QString::fromStdString(firstAccepted->get_name())).arg(moduleId),
                                      QMessageBox::Ok | QMessageBox::Cancel)
                == QMessageBox::Ok)
            {
                ActionAddItemsToObject* act = new ActionAddItemsToObject(mSelectExclude.modules(), mSelectExclude.gates());
                act->setObject(UserActionObject(moduleId, UserActionObjectType::Module));
                act->exec();
                gSelectionRelay->clear();
                gSelectionRelay->addModule(moduleId);
                gSelectionRelay->setFocus(SelectionRelay::ItemType::Module, moduleId);
                gSelectionRelay->relaySelectionChanged(this);
                gContentManager->getGraphTabWidget()->ensureSelectionVisible();

                ModuleSelectHistory::instance()->add(moduleId);
            }
        }
        else if (notAccepted)
            QMessageBox::warning(qApp->activeWindow(), "Warning", QString("Cannot move %1 into module [%2]").arg(mSelectExclude.selectionToString()).arg(notAccepted));
        else
            terminate = false;

        if (terminate)
            terminateCurrentPicker();
    }

    void ModuleSelectPicker::terminateCurrentPicker()
    {
        if (!sCurrentPicker)
            return;
        ModuleSelectPicker* toDelete = sCurrentPicker;
        sCurrentPicker               = nullptr;
        toDelete->triggerCursor(false);
        disconnect(gSelectionRelay, &SelectionRelay::selectionChanged, toDelete, &ModuleSelectPicker::handleSelectionChanged);
        toDelete->deleteLater();
    }

    //---------------- VIEW -------------------------------------------
    ModuleSelectView::ModuleSelectView(bool history, Searchbar* sbar, QWidget* parent) : QTableView(parent)
    {
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);

        ModuleSelectProxy* prox = new ModuleSelectProxy(this);
        connect(sbar, &Searchbar::textEdited, prox, &ModuleSelectProxy::searchTextChanged);

        ModuleSelectModel* modl = new ModuleSelectModel(history, this);
        prox->setSourceModel(modl);
        setModel(prox);

        connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &ModuleSelectView::handleCurrentChanged);
        connect(this, &QTableView::doubleClicked, this, &ModuleSelectView::handleDoubleClick);
        setSortingEnabled(true);
        sortByColumn(history ? -1 : 2, Qt::AscendingOrder);
        resizeColumnsToContents();
        horizontalHeader()->setStretchLastSection(true);
        verticalHeader()->hide();
    }

    void ModuleSelectView::handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);
        const ModuleSelectProxy* prox = static_cast<const ModuleSelectProxy*>(model());
        Q_ASSERT(prox);
        const ModuleSelectModel* modl = static_cast<const ModuleSelectModel*>(prox->sourceModel());
        QModelIndex sourceIndex       = prox->mapToSource(current);
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

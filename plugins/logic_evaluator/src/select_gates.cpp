#include "logic_evaluator/select_gates.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QGridLayout>
#include <QVector>

namespace hal {
    QVariant SelectGateModel::data(const QModelIndex& index, int role) const
    {
        const SelectGateItem* item = dynamic_cast<const SelectGateItem*>(getItemFromIndex(index));

        switch (role)
        {
        case Qt::ForegroundRole:
            if (item && item->getType() == ModuleItem::TreeItemType::Gate && item->state() == Qt::Checked)
                return QColor("#FFE8A0"); //TODO : style file
            break;
        case Qt::BackgroundRole:
            if (item && item->getType() == ModuleItem::TreeItemType::Gate && item->state() == Qt::Checked)
                return QColor("#102030");
            break;
        case Qt::CheckStateRole:
            if (item && !index.column()) return item->state();
            break;
        }
        return ModuleModel::data(index, role);
    }

    bool SelectGateModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role != Qt::CheckStateRole) return false;
        SelectGateItem* item = dynamic_cast<SelectGateItem*>(getItemFromIndex(index));
        if (!item) return false;
        switch(item->getType()) {
        case ModuleItem::TreeItemType::Module:
            setModuleStateRecursion(item, (Qt::CheckState) value.toInt());
            break;
        case ModuleItem::TreeItemType::Gate:
            item->setState((Qt::CheckState) value.toInt());
            setCheckedRecursion(false, mRootItem);
            break;
        case ModuleItem::TreeItemType::Net:
            Q_ASSERT(1==0);
            break;
        }
        return true;
    }

    void SelectGateModel::setModuleStateRecursion(SelectGateItem* item, Qt::CheckState stat)
    {
        item->setState(stat);
        QModelIndex inx0 = getIndexFromItem(item);
        QModelIndex inx1 = createIndex(inx0.row(), 2, inx0.internalPointer());
        Q_EMIT dataChanged(inx0,inx1);
        for (BaseTreeItem* bti : item->getChildren())
        {
            SelectGateItem* child = dynamic_cast<SelectGateItem*>(bti);
            setModuleStateRecursion(child,stat);
        }
    }


    SelectGateModel::SelectGateModel(QObject* parent)
    {
        insertModuleRecursion(gNetlist->get_top_module());
    }

    int SelectGateModel::insertModuleRecursion(const Module* mod, SelectGateItem* parentItem)
    {
        SelectGateItem* child = new SelectGateItem(mod->get_id(), ModuleItem::TreeItemType::Module);
        for (const Module* subm : mod->get_submodules())
            insertModuleRecursion(subm, child);
        for (const Gate* g : mod->get_gates(nullptr, false))
        {
            child->appendChild(new SelectGateItem(g->get_id(), ModuleItem::TreeItemType::Gate));
        }
        if (parentItem)
            parentItem->appendChild(child);
        else
            mRootItem->appendChild(child);
    }

    QPair<bool,bool> SelectGateModel::setCheckedRecursion(bool applySet, BaseTreeItem *parentItem, const QSet<u32> &selectedGateIds)
    {
        bool hasChecked = false;
        bool hasUnchecked = false;
        for (BaseTreeItem* bti : parentItem->getChildren())
        {
            QPair<bool,bool> res = setCheckedRecursion(applySet, bti, selectedGateIds);
            if (res.first) hasChecked = true;
            if (res.second) hasUnchecked = true;
        }
        SelectGateItem* item = dynamic_cast<SelectGateItem*>(parentItem);
        if (item)
        {
            Qt::CheckState lastState;
            switch (item->getType()) {
            case ModuleItem::TreeItemType::Module:
                lastState = item->state();
                if (hasChecked && hasUnchecked)
                    item->setState(Qt::PartiallyChecked);
                else if (hasChecked)
                    item->setState(Qt::Checked);
                else
                    item->setState(Qt::Unchecked);
                if (!applySet && item->state() != lastState)
                {
                    QModelIndex inx = getIndexFromItem(item);
                    Q_EMIT dataChanged(inx,inx);
                }
                break;
            case ModuleItem::TreeItemType::Gate:
                if (applySet)
                {
                    if (selectedGateIds.contains(item->id()))
                    {
                        item->setState(Qt::Checked);
                        hasChecked = true;
                    }
                    else
                    {
                        item->setState(Qt::Unchecked);
                        hasUnchecked = true;
                    }
                }
                else
                {
                    if (item->state() == Qt::Checked)
                       hasChecked = true;
                    else
                       hasUnchecked = true;
                }
                break;
            case ModuleItem::TreeItemType::Net:
                Q_ASSERT(1==0);
                break;
            }
        }
        return QPair<bool,bool>(hasChecked,hasUnchecked);
    }

    void SelectGateModel::setChecked(const std::vector<Gate *> &gates)
    {
        QSet<u32> selectedGateIds;

        for (const Gate* g : gates)
            selectedGateIds.insert(g->get_id());

        setCheckedRecursion(true, mRootItem, selectedGateIds);
    }

    Qt::ItemFlags SelectGateModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags retval = ModuleModel::flags(index);
        if (index.column()) return retval;
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    }

    SelectGates::SelectGates(const std::vector<Gate *>& gates, QWidget* parent)
        : QDialog(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        mTreeView = new QTreeView(this);
        SelectGateModel* model = new SelectGateModel(this);
        model->setChecked(gates);
        mTreeView->setModel(model);
        mTreeView->expandAll();
        layout->addWidget(mTreeView);
    }
}

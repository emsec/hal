#include "logic_evaluator/logic_evaluator_select_gates.h"
#include "logic_evaluator/logic_evaluator_dialog.h"
#include "logic_evaluator/plugin_logic_evaluator.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/searchbar/searchbar.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVector>

namespace hal {
    QVariant SelectGateModel::data(const QModelIndex& index, int role) const
    {
        const SelectGateItem* item = dynamic_cast<const SelectGateItem*>(getItemFromIndex(index));

        switch (role)
        {
        case Qt::ForegroundRole:
            if (item && item->getType() == ModuleItem::TreeItemType::Gate && item->state() == Qt::Checked)
            {
                if (mParentDialog)
                    return mParentDialog->selForeground();
                else
                    return ModuleModel::data(index, role);
            }
            break;
        case Qt::BackgroundRole:
            if (item && item->getType() == ModuleItem::TreeItemType::Gate && item->state() == Qt::Checked)
            {
                if (mParentDialog)
                    return mParentDialog->selBackground();
                else
                    return ModuleModel::data(index, role);
            }
            break;
        case Qt::CheckStateRole:
            if (!item->isSelectable()) return QVariant();
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
        bool stateWasEmpty = mSelectedGates.empty();
        setSelectedGatesRecursion();
        if (stateWasEmpty != mSelectedGates.empty())
            Q_EMIT selectionStateChanged(mSelectedGates.empty());
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
            if (!child->isSelectable()) continue;
            setModuleStateRecursion(child,stat);
        }
    }


    SelectGateModel::SelectGateModel(QObject* parent)
        :    mParentDialog(dynamic_cast<LogicEvaluatorSelectGates*>(parent))
    {
        insertModuleRecursion(gNetlist->get_top_module());
    }

    int SelectGateModel::insertModuleRecursion(const Module* mod, SelectGateItem* parentItem)
    {
        int countCheckable = 0;
        SelectGateItem* child = new SelectGateItem(mod->get_id(), ModuleItem::TreeItemType::Module);
        for (const Module* subm : mod->get_submodules())
            countCheckable += insertModuleRecursion(subm, child);
        for (const Gate* g : mod->get_gates(nullptr, false))
        {
            bool isSel = GuiExtensionLogicEvaluator::acceptGate(g);
            child->appendChild(new SelectGateItem(g->get_id(), ModuleItem::TreeItemType::Gate, isSel));
            if (isSel) ++countCheckable;
        }
        if (!countCheckable) child->setSelectable(false);
        if (parentItem)
            parentItem->appendChild(child);
        else
            mRootItem->appendChild(child);
        return countCheckable;
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
            if (!item->isSelectable()) return QPair<bool,bool>(false,false);
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

    void SelectGateModel::setSelectedGatesRecursion(SelectGateItem* item)
    {
        if (!item)
            mSelectedGates.clear();

        BaseTreeItem* parentItem = nullptr;
        if (item)
        {
            if (!item->isSelectable()) return;
            switch (item->getType()) {
            case ModuleItem::TreeItemType::Module:
                parentItem = item;
                break;
            case ModuleItem::TreeItemType::Gate:
                if (item->state() == Qt::Checked)
                {
                    Gate*g = gNetlist->get_gate_by_id(item->id());
                    if (g) mSelectedGates.push_back(g);
                }
                break;
            case ModuleItem::TreeItemType::Net:
                Q_ASSERT(1==0);
                break;
            }
        }
        else
            parentItem = mRootItem;

        if (parentItem)
            for (BaseTreeItem* childItem : parentItem->getChildren())
                setSelectedGatesRecursion(static_cast<SelectGateItem*>(childItem));
    }

    void SelectGateModel::setChecked(const std::vector<Gate *> &gates)
    {
        QSet<u32> selectedGateIds;

        for (const Gate* g : gates)
            selectedGateIds.insert(g->get_id());

        setCheckedRecursion(true, mRootItem, selectedGateIds);
        setSelectedGatesRecursion();
    }

    Qt::ItemFlags SelectGateModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags retval = ModuleModel::flags(index);
        if (index.column()) return retval;
        const SelectGateItem* item = dynamic_cast<const SelectGateItem*>(getItemFromIndex(index));
        if (!item->isSelectable()) return Qt::NoItemFlags;
        return retval | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    }

    LogicEvaluatorSelectGates::LogicEvaluatorSelectGates(const std::vector<Gate *>& gates, QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("Select combinatorical gates for logic evaluator");
        QGridLayout* layout = new QGridLayout(this);
        mTreeView = new QTreeView(this);
        mSelectGateModel = new SelectGateModel(this);
        mProxyModel = new ModuleProxyModel(this);
        mProxyModel->setSourceModel(mSelectGateModel);
        mProxyModel->toggleFilterGates();
        mSelectGateModel->setChecked(gates);
        mTreeView->setModel(mProxyModel);
        mTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mTreeView->expandAll();
        mTreeView->setColumnWidth(0,250);
        mTreeView->setColumnWidth(1,40);
        mTreeView->setColumnWidth(2,110);

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this , &QDialog::reject);
        mButtonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(mSelectGateModel, &SelectGateModel::selectionStateChanged, this, &LogicEvaluatorSelectGates::handleSelectionStateChanged);
        mButtonBox->button(QDialogButtonBox::Ok)->setDisabled(mSelectGateModel->selectedGates().empty());

        mSearchbar = new Searchbar(this);
        connect(mSearchbar, &Searchbar::triggerNewSearch, mProxyModel, &ModuleProxyModel::startSearch);

        mCompile = new QCheckBox("Compile selected logic", this);
        mCompile->setChecked(true);

        layout->addWidget(mTreeView,0,0,1,2);
        layout->addWidget(mSearchbar, 1,0,1,2);
        layout->addWidget(mCompile, 2,0,1,2);
        layout->addWidget(mButtonBox, 3,1);
        mTreeView->setMinimumWidth(400);

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
    }

    void LogicEvaluatorSelectGates::handleSelectionStateChanged(bool empty)
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setDisabled(empty);
    }

    void LogicEvaluatorSelectGates::accept()
    {
        LogicEvaluatorDialog* led = new LogicEvaluatorDialog(mSelectGateModel->selectedGates(),!mCompile->isChecked());
        led->show();
        led->raise();
        QDialog::accept();
    }
}

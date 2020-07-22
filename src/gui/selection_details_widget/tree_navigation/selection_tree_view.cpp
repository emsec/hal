#include <QHeaderView>
#include "selection_details_widget/tree_navigation/selection_tree_view.h"
#include "selection_details_widget/tree_navigation/selection_tree_model.h"

namespace hal {
    SelectionTreeView::SelectionTreeView(QWidget *parent)
        : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mSelectionTreeModel = new SelectionTreeModel(this);
        setModel(mSelectionTreeModel);
        setDefaultColumnWidth();
    }

    void SelectionTreeView::setDefaultColumnWidth()
    {
        setColumnWidth(SelectionTreeModel::ID_COLUMN, 40);
        setColumnWidth(SelectionTreeModel::TYPE_COLUMN, 80);
        header()->setSectionResizeMode(SelectionTreeModel::NAME_COLUMN, QHeaderView::Stretch);
        header()->setSectionResizeMode(SelectionTreeModel::ID_COLUMN, QHeaderView::Interactive);
        header()->setSectionResizeMode(SelectionTreeModel::TYPE_COLUMN, QHeaderView::Interactive);
    }

    void SelectionTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);

        SelectionTreeItem::itemType_t tp = SelectionTreeItem::NullItem;
        u32 id = 0;

        const SelectionTreeItem* sti = current.isValid()
                ? static_cast<const SelectionTreeItem*>(mSelectionTreeModel->itemFromIndex(current))
                : nullptr;

        if (sti)
        {
            tp = sti->itemType();
            id = sti->id();
        }
        Q_EMIT triggerSelection(tp,id);
    }

    void SelectionTreeView::populate(bool visible)
    {
        setSelectionMode(QAbstractItemView::NoSelection);
        selectionModel()->clear();
        mSelectionTreeModel->fetchSelection(visible);
        if (visible)
            show();
        else
            hide();
        setSelectionMode(QAbstractItemView::SingleSelection);
    }
}

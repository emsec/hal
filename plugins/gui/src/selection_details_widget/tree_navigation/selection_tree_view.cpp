#include <QHeaderView>
#include "selection_details_widget/tree_navigation/selection_tree_view.h"
#include "selection_details_widget/tree_navigation/selection_tree_model.h"

namespace hal {
    SelectionTreeView::SelectionTreeView(QWidget *parent)
        : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_selectionTreeModel = new SelectionTreeModel(this);
        setModel(m_selectionTreeModel);
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

        const SelectionTreeItem* sti = current.isValid()
                ? static_cast<const SelectionTreeItem*>(m_selectionTreeModel->itemFromIndex(current))
                : nullptr;

        Q_EMIT triggerSelection(sti);
    }

    void SelectionTreeView::populate(bool visible)
    {
        setSelectionMode(QAbstractItemView::NoSelection);
        selectionModel()->clear();
        m_selectionTreeModel->fetchSelection(visible);
        if (visible)
        {
            show();
            setSelectionMode(QAbstractItemView::SingleSelection);
            QModelIndex defaultSel = m_selectionTreeModel->defaultIndex();
            if (defaultSel.isValid())
                selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        else
            hide();
    }
}

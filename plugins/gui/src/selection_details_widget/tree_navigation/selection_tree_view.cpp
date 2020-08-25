#include <QHeaderView>
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"

namespace hal {
    SelectionTreeView::SelectionTreeView(QWidget *parent)
        : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_selectionTreeModel = new SelectionTreeModel(this);
        m_selectionTreeProxyModel = new SelectionTreeProxyModel(this);
        m_selectionTreeProxyModel->setSourceModel(m_selectionTreeModel);
        setModel(m_selectionTreeProxyModel);
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
                ? itemFromIndex(current)
                : nullptr;

        Q_EMIT triggerSelection(sti);
    }


    SelectionTreeItem* SelectionTreeView::itemFromIndex(const QModelIndex& index) const
    {
        // topmost element if no valid index given
        QModelIndex proxyIndex = index.isValid()
                ? index
                : m_selectionTreeProxyModel->index(0,0,rootIndex());

        if (!proxyIndex.isValid()) return nullptr;

        QModelIndex modelIndex = m_selectionTreeProxyModel->mapToSource(proxyIndex);
        return static_cast<SelectionTreeItem*>(modelIndex.internalPointer());
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
            QModelIndex defaultSel = m_selectionTreeProxyModel->index(0,0,rootIndex());
            if (defaultSel.isValid())
                selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        else
            hide();
    }

    void SelectionTreeView::handle_filter_text_changed(const QString& filter_text)
    {
        m_selectionTreeProxyModel->handle_filter_text_changed(filter_text);
        QModelIndex defaultSel = m_selectionTreeProxyModel->index(0,0,rootIndex());
        if (defaultSel.isValid())
            selectionModel()->setCurrentIndex(defaultSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

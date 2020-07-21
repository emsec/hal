#pragma once

#include <QWidget>
#include <QTreeView>
#include "selection_details_widget/tree_navigation/selection_tree_item.h"
#include "selection_details_widget/tree_navigation/selection_tree_model.h"

namespace hal {

    class SelectionTreeView : public QTreeView
    {
        Q_OBJECT

    Q_SIGNALS:
        void triggerSelection(SelectionTreeItem::itemType_t t, u32 id);

    protected:
        virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous) Q_DECL_OVERRIDE;

    public:
        SelectionTreeView(QWidget* parent = nullptr);
        void setDefaultColumnWidth();
        void clearHide();
        void populate(bool visible);

    private:
        SelectionTreeModel*  mSelectionTreeModel;
    };
}

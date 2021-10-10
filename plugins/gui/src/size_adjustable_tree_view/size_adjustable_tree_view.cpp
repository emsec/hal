#include "gui/size_adjustable_tree_view/size_adjustable_tree_view.h"
#include "gui/basic_tree_model/base_tree_model.h"
#include <QHeaderView>
#include <QQueue>

namespace hal
{
    SizeAdjustableTreeView::SizeAdjustableTreeView(QWidget *parent) : QTreeView(parent)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        connect(this, &QTreeView::collapsed, this, &SizeAdjustableTreeView::handleExpandedOrCollapsed);
        connect(this, &QTreeView::expanded, this, &SizeAdjustableTreeView::handleExpandedOrCollapsed);
    }

    void SizeAdjustableTreeView::adjustSizeToContents()
    {
        if(!model() || !static_cast<BaseTreeModel*>(model()))
            return;

        auto baseModel = static_cast<BaseTreeModel*>(model());

        header()->setStretchLastSection(false); //hacky (needed so resizeColumnsToContent functions as intended)
        for(int i = 0; i < baseModel->columnCount(); i++)
            resizeColumnToContents(i);


        int rows = baseModel->rowCount();
        int w = 0;
        int h = header()->height();

        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the table's width far too big, so just set 0 as the size

        if(rows != 0)
        {
            for (int i = 0; i < baseModel->columnCount(); i++)
                w += columnWidth(i);    // seems to include gridline

            //HEIGHT CALCULATION
            QQueue<TreeItem*> queue;
            for(auto item : baseModel->getRootItem()->getChildren())
                queue.enqueue(item);

            //int indexRowHeight = indexRowSizeHint(baseModel->getIndexFromItem(baseModel->getRootItem()->getChild(0)));
            int indexRowHeight = rowHeight(baseModel->getIndexFromItem(baseModel->getRootItem()->getChild(0)));

            while(!queue.isEmpty())
            {
                TreeItem* currItem = queue.dequeue();
                QModelIndex itemIndex = baseModel->getIndexFromItem(currItem);
                h += indexRowHeight;

                if(isExpanded(itemIndex))
                    for(auto childItem : currItem->getChildren())
                        queue.enqueue(childItem);
            }

            //one for the team (must be added for some additional size so the invisible scrossbars!
            h += (indexRowHeight/2)-6;
        }

        setFixedHeight(h);
        setMinimumWidth(w);

        header()->setStretchLastSection(true); //hacky (after calculation the header can be stretched again)

        updateGeometry();
        update();


    }

    void SizeAdjustableTreeView::setModel(QAbstractItemModel *model)
    {
        if(this->model())
        {
            disconnect(this->model(), &QAbstractItemModel::rowsInserted, this, &SizeAdjustableTreeView::handleRowsInsertedOrRemoved);
            disconnect(this->model(), &QAbstractItemModel::rowsRemoved, this, &SizeAdjustableTreeView::handleRowsInsertedOrRemoved);
            disconnect(this->model(), &QAbstractItemModel::modelReset, this, &SizeAdjustableTreeView::handleModelReset);
        }

        QTreeView::setModel(model);//all normal view connects

        //own connects, triggered after all connects in setModel (hopefully)
        if(!model) return; //dont connect a nullptr
        connect(model, &QAbstractItemModel::rowsInserted, this, &SizeAdjustableTreeView::handleRowsInsertedOrRemoved);
        connect(model, &QAbstractItemModel::rowsRemoved, this, &SizeAdjustableTreeView::handleRowsInsertedOrRemoved);
        connect(model, &QAbstractItemModel::modelReset, this, &SizeAdjustableTreeView::handleModelReset);
    }

    void SizeAdjustableTreeView::handleExpandedOrCollapsed(const QModelIndex &index)
    {
        Q_UNUSED(index)
        adjustSizeToContents();
    }

    void SizeAdjustableTreeView::handleRowsInsertedOrRemoved(const QModelIndex &parent, int first, int last)
    {
        Q_UNUSED(parent)
        Q_UNUSED(first)
        Q_UNUSED(last)
        adjustSizeToContents();
    }

    void SizeAdjustableTreeView::handleModelReset()
    {
        adjustSizeToContents();
    }

}

#include "gui/selection_details_widget/general_table_widget.h"

#include <QMenu>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>

namespace hal
{
    GeneralTableWidget::GeneralTableWidget(QWidget* parent) : QTableWidget(parent)
    {
        setColumnCount(2);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::NoSelection);
        setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalHeader()->hide();
        verticalHeader()->hide();
        verticalHeader()->setDefaultSectionSize(16);
        setShowGrid(false);
        setFocusPolicy(Qt::NoFocus);
        setFrameStyle(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setContextMenuPolicy(Qt::CustomContextMenu);

        //setMaximumHeight(verticalHeader()->length());
        //resizeColumnToContents(0);
        //mGeneralView->resizeColumnsToContents();
        //mGeneralView->setFixedSize(DetailsTableUtilities::tableViewSize(mGeneralView,mGeneralModel->rowCount(),mGeneralModel->columnCount()));

        connect(this, &GeneralTableWidget::customContextMenuRequested, this, &GeneralTableWidget::serveContextMenu);
        connect(this, &GeneralTableWidget::doubleClicked, this, &GeneralTableWidget::handleDoubleClick);
    }

    void GeneralTableWidget::setRow(const QString& key, const QString& val, QMenu* contextMenu, std::function<void()> doubleClickAction)
    {
        if(mKeyItemMap.contains(key))
        {
            QTableWidgetItem* valItem = mKeyItemMap.value(key);
            
            valItem->setText(val);

            if(contextMenu == nullptr)
            {
                int rowIndex = valItem->row();
                mRowMenuMap.remove(rowIndex);
            }

            if(doubleClickAction == nullptr)
            {
                int rowIndex = valItem->row();
                mRowDoubleClickActionMap.remove(rowIndex);
            }
        }
        else
        {
            int rowIndex = rowCount();

            setRowCount(rowCount() + 1);

            QTableWidgetItem* keyItem = new QTableWidgetItem(key);
            QTableWidgetItem* valItem = new QTableWidgetItem(val);

            setItem(rowIndex, 0, keyItem);
            setItem(rowIndex, 1, valItem);

            mKeyItemMap.insert(key, valItem);

            if(contextMenu != nullptr)
                if(!mRowMenuMap.contains(rowIndex))
                    mRowMenuMap.insert(rowIndex, contextMenu);

            if(doubleClickAction != nullptr)
                if(!mRowDoubleClickActionMap.contains(rowIndex))
                    mRowDoubleClickActionMap.insert(rowIndex, doubleClickAction);
        }
    }

    void GeneralTableWidget::serveContextMenu(const QPoint& point) const
    {
        const QModelIndex clickedIndex = indexAt(point);

        if(clickedIndex.isValid())
        {
            int row = clickedIndex.row();

            if(mRowMenuMap.contains(row))
            {
                QMenu* menu = mRowMenuMap.value(clickedIndex.row());
                menu->exec(viewport()->mapToGlobal(point));
            }
        }
    }

    void GeneralTableWidget::handleDoubleClick(const QModelIndex& index) const
    {
        if(index.isValid())
        {
            int row = index.row();

            if(mRowDoubleClickActionMap.contains(row))
            {
                std::function<void()> action = mRowDoubleClickActionMap.value(row);
                action();
            }
        }
    }

    void GeneralTableWidget::adjustSize()
    {
        resizeColumnsToContents();

        int rows = rowCount();
        int columns = columnCount();

        int w = 0;
        int h = 0;

        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the table's width far too big, so just set 0 as the size

        if(rows != 0)
        {
            w = verticalHeader()->width() + 4;    // +4 seems to be needed

            for (int i = 0; i < columns; i++)
                w += columnWidth(i);    // seems to include gridline


            h = horizontalHeader()->height() + 4;

            for (int i = 0; i < rows; i++)
                h += rowHeight(i);

            w = w + 5; //no contemporary source exists why 5 is the magic number here (my guess would be it's the width of the hidden scrollbar)
        }

        setFixedSize(QSize(w, h)); 
    }

    void GeneralTableWidget::copyToClipboard(const QString& text) const
    {
        QApplication::clipboard()->setText(text);
    }
}

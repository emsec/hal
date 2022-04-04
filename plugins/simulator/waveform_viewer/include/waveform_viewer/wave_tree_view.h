#pragma once

#include <QTreeView>
#include <QHash>
#include <QResizeEvent>
#include <QStyledItemDelegate>
#include "hal_core/defines.h"
#include "waveform_viewer/wave_item.h"

namespace hal {
    class WaveDataList;

    class WaveValueDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const override;
    };

    class WaveTreeView : public QTreeView
    {
        Q_OBJECT
        QList<QModelIndex> mItemOrder;
        QModelIndexList mContextIndexList;
        WaveDataList* mWaveDataList;
        WaveItemHash* mWaveItemHash;

        QModelIndexList sortedSelection() const;
        void editWaveData(WaveData* wd);

    protected:
        void mouseDoubleClickEvent(QMouseEvent *event) override;
        void startDrag(Qt::DropActions supportedActions) override;
        void scrollContentsBy(int dx, int dy) override;
        bool viewportEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    Q_SIGNALS:
        void viewportHeightChanged(int height);
        void sizeChanged(int viewportHeight, int scrollbarMax, int scrollbarPos);
        void numberVisibleChanged(int nVisible, int scrollbarMax, int scrollbarPos);
        void triggerUpdateWaveItems();

    private Q_SLOTS:
        void handleExpandCollapse(const QModelIndex& index);
        void handleContextMenuRequested(const QPoint& pos);
        void handleRemoveItem();
        void handleRenameItem();
        void handleEditOrBrowseItem();
        void handleRemoveGroup();
        void handleInsertGroup();
        void handleInsertBoolean();
        void handleSetValueFormat();
        void handleRemoveMulti();

    public Q_SLOTS:
        void handleInserted(const QModelIndex& index);
        void reorder();
        void setWaveSelection(const QSet<u32>& netIds);

    public:
        WaveTreeView(WaveDataList* wdList, WaveItemHash* wHash, QWidget* parent = nullptr);

    private:
        void orderRecursion(const QModelIndex& parent);
    };
}

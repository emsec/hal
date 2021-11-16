#pragma once

#include <QTreeView>
#include <QHash>
#include <QResizeEvent>
#include "hal_core/defines.h"

namespace hal {
    class WaveDataList;

    class WaveTreeView : public QTreeView
    {
        Q_OBJECT
        QList<QModelIndex> mItemOrder;
        QModelIndex mContextIndex;
        WaveDataList* mWaveDataList;

    protected:
        void mouseDoubleClickEvent(QMouseEvent *event) override;
        void startDrag(Qt::DropActions supportedActions) override;
        void scrollContentsBy(int dx, int dy) override;
        bool viewportEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    Q_SIGNALS:
        void viewportHeightChanged(int height);
        void sizeChanged(int widgetHeight, int viewportHeight);
        void reordered(QHash<int,int> wavePosition, QHash<int,int> groupPosition);
        void valueBaseChanged();

    private Q_SLOTS:
        void handleExpandCollapse(const QModelIndex& index);
        void handleContextMenuRequested(const QPoint& pos);
        void handleRemoveItem();
        void handleRenameItem();
        void handleEditOrBrowseItem();
        void handleRemoveGroup();
        void handleInsertGroup();
        void handleSetValueFormat();

    public Q_SLOTS:
        void handleInserted(const QModelIndex& index);
        void reorder();
        void setWaveSelection(const QSet<u32>& netIds);

    public:
        WaveTreeView(WaveDataList* wdList, QWidget* parent = nullptr);

    private:
        void orderRecursion(const QModelIndex& parent);
    };
}

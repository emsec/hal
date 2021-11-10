#pragma once

#include <QTreeView>
#include <QHash>
#include <QSet>
#include <QResizeEvent>
#include "hal_core/defines.h"

namespace hal {
    class WaveDataList;
    class VolatileWaveData;

    class WaveTreeView : public QTreeView
    {
        Q_OBJECT
        QList<QModelIndex> mOrder;
        QModelIndex mContextIndex;
        WaveDataList* mWaveDataList;
        VolatileWaveData* mVolatileWaveData;

    protected:
        void mouseDoubleClickEvent(QMouseEvent *event) override;
        void startDrag(Qt::DropActions supportedActions) override;
        void scrollContentsBy(int dx, int dy) override;
        bool viewportEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    Q_SIGNALS:
        void viewportHeightChanged(int height);
        void sizeChanged(int widgetHeight, int viewportHeight);
        void reordered(QHash<int,int> wavePosition);

    private Q_SLOTS:
        void handleExpandCollapse(const QModelIndex& index);
        void handleContextMenuRequested(const QPoint& pos);
        void handleRemoveItem();
        void handleRemoveGroup();
        void handleInsertGroup();

    public Q_SLOTS:
        void handleInserted(const QModelIndex& index);
        void reorder();

    public:
        WaveTreeView(WaveDataList* wdList, VolatileWaveData* wdVol, QWidget* parent = nullptr);
        void setWaveSelection(const QSet<u32>& netIds);

    private:
        void orderRecursion(const QModelIndex& parent);
    };
}

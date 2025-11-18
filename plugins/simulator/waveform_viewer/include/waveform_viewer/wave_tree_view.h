// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
        QSet<u32> mExpandedGroups;
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
//        void numberVisibleChanged(int nVisible, int scrollbarMax, int scrollbarPos);  // unused
        void triggerUpdateWaveItems();

    private Q_SLOTS:
        void handleExpand(const QModelIndex& index);
        void handleCollapse(const QModelIndex& index);
        void handleContextMenuRequested(const QPoint& pos);
        void handleRemoveItem();
        void handleRenameItem();
        void handleEditOrBrowseItem();
        void handleRemoveGroup();
        void handleInsertGroup();
        void handleInsertBoolean();
        void handleInsertTrigger();
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

//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "gui/content_widget/content_widget.h"

#include "hal_core/defines.h"

#include "gui/grouping/grouping_table_model.h"

#include <QListWidget>
#include <QPoint>
#include <QTableView>
#include <QPushButton>
#include <QMenu>
#include <QMap>
#include <QColor>

namespace hal
{
    class GraphTabWidget;
    class Grouping;
    class GroupingProxyModel;
    class Searchbar;

    class GroupingManagerWidget : public ContentWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString newGroupingIconPath READ newGroupingIconPath WRITE setNewGroupingIconPath)
        Q_PROPERTY(QString newGroupingIconStyle READ newGroupingIconStyle WRITE setNewGroupingIconStyle)
        Q_PROPERTY(QString renameGroupingIconPath READ renameGroupingIconPath WRITE setRenameGroupingIconPath)
        Q_PROPERTY(QString renameGroupingIconStyle READ renameGroupingIconStyle WRITE setRenameGroupingIconStyle)
        Q_PROPERTY(QString deleteIconPath READ deleteIconPath WRITE setDeleteIconPath)
        Q_PROPERTY(QString deleteIconStyle READ deleteIconStyle WRITE setDeleteIconStyle)
        // Q_PROPERTY(QString openIconPath READ openIconPath WRITE setOpenIconPath)
        // Q_PROPERTY(QString openIconStyle READ openIconStyle WRITE setOpenIconStyle)
        Q_PROPERTY(QString colorSelectIconPath READ colorSelectIconPath WRITE setColorSelectIconPath)
        Q_PROPERTY(QString colorSelectIconStyle READ colorSelectIconStyle WRITE setColorSelectIconStyle)
        Q_PROPERTY(QString toSelectionIconPath READ toSelectionIconPath WRITE setToSelectionIconPath)
        Q_PROPERTY(QString toSelectionIconStyle READ toSelectionIconStyle WRITE setToSelectionIconStyle)

    public:
        GroupingManagerWidget(GraphTabWidget* tab_view, QWidget* parent = nullptr);

        virtual void setupToolbar(Toolbar* toolbar) Q_DECL_OVERRIDE;
        virtual QList<QShortcut*> createShortcuts() override;

        QString newGroupingIconPath() const;
        QString newGroupingIconStyle() const;
        QString renameGroupingIconPath() const;
        QString renameGroupingIconStyle() const;
        QString deleteIconPath() const;
        QString deleteIconStyle() const;
        // QString openIconPath() const;
        // QString openIconStyle() const;
        QString colorSelectIconPath() const;
        QString colorSelectIconStyle() const;
        QString toSelectionIconPath() const;
        QString toSelectionIconStyle() const;
        QString disabledIconStyle() const;

        void setDisabledIconStyle(const QString &style);
        void setNewGroupingIconPath(const QString &path);
        void setNewGroupingIconStyle(const QString &style);
        void setRenameGroupingIconPath(const QString &path);
        void setRenameGroupingIconStyle(const QString &style);
        void setDeleteIconPath(const QString &path);
        void setDeleteIconStyle(const QString &style);
        // void setOpenIconPath(const QString &path);
        // void setOpenIconStyle(const QString &style);
        void setColorSelectIconPath(const QString &path);
        void setColorSelectIconStyle(const QString &style);
        void setToSelectionIconPath(const QString &path);
        void setToSelectionIconStyle(const QString &style);

        GroupingTableModel* getModel() const { return mGroupingTableModel; }

    public Q_SLOTS:
        void handleLastEntryDeleted();
        void handleNewEntryAdded(const QModelIndex& modelIndex);
        void handleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
        void handleCurrentChanged(const QModelIndex &current = QModelIndex(), const QModelIndex &previous = QModelIndex());

    private Q_SLOTS:
        void toggleSearchbar();
        void filter(const QString& text);
        void handleCreateGroupingClicked();
        void handleRenameGroupingClicked();
        void handleColorSelectClicked();
        void handleToSelectionClicked();
        void handleDeleteGroupingClicked();

        void handleContextMenuRequest(const QPoint& point);

    private:
        GraphTabWidget* mTabView;
        QTableView* mGroupingTableView;
        GroupingTableModel* mGroupingTableModel;
        GroupingProxyModel* mProxyModel;
        Searchbar* mSearchbar;

        QAction* mNewGroupingAction;
        QString mNewGroupingIconPath;
        QString mNewGroupingIconStyle;

        QAction* mRenameAction;
        QString mRenameGroupingIconPath;
        QString mRenameGroupingIconStyle;

        QAction* mColorSelectAction;
        QString mColorSelectIconPath;
        QString mColorSelectIconStyle;

        QAction* mDeleteAction;
        QString mDeleteIconPath;
        QString mDeleteIconStyle;

        QAction* mToSelectionAction;
        QString mToSelectionIconPath;
        QString mToSelectionIconStyle;

        QString mDisabledIconStyle;

        void setToolbarButtonsEnabled(bool enabled);

        GroupingTableEntry getCurrentGrouping();
    };
}

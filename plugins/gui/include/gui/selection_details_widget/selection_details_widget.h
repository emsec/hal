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
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "hal_core/defines.h"

class QTableWidget;
class QStackedWidget;
class QSplitter;
class QListWidget;
class QLabel;

namespace hal
{
    class Grouping;
    class Searchbar;
    class SelectionTreeView;
    class ModuleDetailsWidget;
    class GateDetailsWidget;
    class NetDetailsWidget;
    class SelectionHistoryNavigator;

    class SelectionDetailsWidget : public ContentWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString searchIconPath READ searchIconPath WRITE setSearchIconPath)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString searchActiveIconStyle READ searchActiveIconStyle WRITE setSearchActiveIconStyle)
        Q_PROPERTY(QString restoreIconPath READ restoreIconPath WRITE setRestoreIconPath)
        Q_PROPERTY(QString restoreIconStyle READ restoreIconStyle WRITE setRestoreIconStyle)
        Q_PROPERTY(QString toGroupingIconPath READ toGroupingIconPath WRITE setToGroupingIconPath)
        Q_PROPERTY(QString toGroupingIconStyle READ toGroupingIconStyle WRITE setToGroupingIconStyle)
        Q_PROPERTY(QString toModuleIconPath READ toModuleIconPath WRITE setToModuleIconPath)
        Q_PROPERTY(QString toModuleIconStyle READ toModuleIconStyle WRITE setToModuleIconStyle)

    public:
        SelectionDetailsWidget(QWidget* parent = 0);
        void clear();

        virtual void setupToolbar(Toolbar* toolbar) Q_DECL_OVERRIDE;

        QString disabledIconStyle() const;

        QString searchIconPath() const;
        QString searchIconStyle() const;
        QString searchActiveIconStyle() const;

        QString restoreIconPath() const;
        QString restoreIconStyle() const;

        QString toGroupingIconPath() const;
        QString toGroupingIconStyle() const;

        QString toModuleIconPath() const;
        QString toModuleIconStyle() const;
                
        void setDisabledIconStyle(const QString &style);

        void setSearchIconPath(const QString &path);
        void setSearchIconStyle(const QString &style);
        void setSearchActiveIconStyle(const QString &style);

        void setRestoreIconPath(const QString &path);
        void setRestoreIconStyle(const QString &style);

        void setToGroupingIconPath(const QString &path);
        void setToGroupingIconStyle(const QString &style);
        
        void setToModuleIconPath(const QString &path);
        void setToModuleIconStyle(const QString &style);
        
        void selectionToGroupingInternal(Grouping* grp);

    Q_SIGNALS:
        void triggerHighlight(QVector<const SelectionTreeItem*> highlight);

    public Q_SLOTS:
        void handleSelectionUpdate(void* sender);
        void handleTreeSelection(const SelectionTreeItem* sti);
        QList<QShortcut*> createShortcuts() Q_DECL_OVERRIDE;


    private Q_SLOTS:
        void restoreLastSelection();
        void selectionToGrouping();
        void selectionToModuleMenu();
        void toggleSearchbar();
        void selectionToNewGrouping();
        void selectionToExistingGrouping();
        void selectionToModuleAction();

    private:
        void singleSelectionInternal(const SelectionTreeItem* sti);

        QSplitter*           mSplitter;
        SelectionTreeView*   mSelectionTreeView;
        QWidget*             mSelectionDetails;
        unsigned int         mNumberSelectedItems;

        QStackedWidget* mStackedWidget;

        GateDetailsWidget* mGateDetails;
        NetDetailsWidget* mNetDetails;
        ModuleDetailsWidget* mModuleDetails;
        QLabel* mItemDeletedLabel;
        QLabel* mNoSelectionLabel;

        Searchbar* mSearchbar;

        QAction* mRestoreLastSelection;
        QAction* mSelectionToGrouping;
        QAction* mSelectionToModule;
        QAction* mSearchAction;

        QString mDisabledIconStyle;

        QString mSearchIconPath;
        QString mSearchIconStyle;
        QString mSearchActiveIconStyle;

        QString mRestoreIconPath;
        QString mRestoreIconStyle;

        QString mToGroupingIconPath;
        QString mToGroupingIconStyle;
        
        QString mToModuleIconPath;
        QString mToModuleIconStyle;
        
        SelectionHistoryNavigator* mHistory;

        void handleFilterTextChanged(const QString& filter_text);
        void canRestoreSelection();
        void canMoveToModule(int nodes);
        void enableSearchbar(bool enable);

        static const QString sAddToGrouping;
    };
}

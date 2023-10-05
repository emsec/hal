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

#include "gui/content_widget/content_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "hal_core/defines.h"
#include "gui/settings/settings_items/settings_item_keybind.h"

#include <QColor>
#include "gui/grouping/grouping_color_serializer.h"
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QTableView>

namespace hal
{
    class GraphTabWidget;
    class Grouping;
    class GroupingProxyModel;
    class Searchbar;
    class GraphicsItem;

    /**
     * @ingroup utility_widgets-grouping
     * @brief User interface for Groupings.
     *
     * Groupings are disjoint subsets of Gate%s, Module%s and Net%s that can be used to categorize these objects. In the
     * GUI the groupings are drawn in their specified color. The objects of one grouping can be easily selected
     * all together to simplify the work with gates/modules/nets that share a certain property.
     */
    class GroupingManagerWidget : public ContentWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString newGroupingIconPath READ newGroupingIconPath WRITE setNewGroupingIconPath)
        Q_PROPERTY(QString newGroupingIconStyle READ newGroupingIconStyle WRITE setNewGroupingIconStyle)
        Q_PROPERTY(QString toolboxIconPath READ toolboxIconPath WRITE setToolboxIconPath)
        Q_PROPERTY(QString toolboxIconStyle READ toolboxIconStyle WRITE setToolboxIconStyle)
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
        Q_PROPERTY(QString searchIconPath READ searchIconPath WRITE setSearchIconPath)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString searchActiveIconStyle READ searchActiveIconStyle WRITE setSearchActiveIconStyle)

        Q_PROPERTY(QString tableIconPath READ tableIconPath WRITE setTableIconPath)
        Q_PROPERTY(QString tableIconStyle READ tableIconStyle WRITE setTableIconStyle)

    public:
        /**
        * Constructor.
        *
        * @param parent - The parent widget
        */
        GroupingManagerWidget(QWidget* parent = nullptr);

        /**
         * Setups the toolbar with the actions that are supported by the grouping.
         *
         * @param toolbar - The toolbar to set up
         */
        virtual void setupToolbar(Toolbar* toolbar) override;

        /**
         * Create and connect the shortcuts that were provided by the GroupingsManagerWidget
         *
         * @returns the list of created shortcuts.
         */
        virtual QList<QShortcut*> createShortcuts() override;

        /** @name Q_PROPERTY READ Functions
         */
        ///@{
        QString newGroupingIconPath() const;
        QString newGroupingIconStyle() const;
        QString toolboxIconPath() const;
        QString toolboxIconStyle() const;
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
        QString searchIconPath() const;
        QString searchIconStyle() const;
        QString searchActiveIconStyle() const;

        QString tableIconPath() const;
        QString tableIconStyle() const;
        ///@}

        /** @name Q_PROPERTY WRITE Functions
         */
        ///@{
        void setDisabledIconStyle(const QString& style);
        void setNewGroupingIconPath(const QString& path);
        void setNewGroupingIconStyle(const QString& style);
        void setToolboxIconPath(const QString& path);
        void setToolboxIconStyle(const QString& style);
        void setRenameGroupingIconPath(const QString& path);
        void setRenameGroupingIconStyle(const QString& style);
        void setDeleteIconPath(const QString& path);
        void setDeleteIconStyle(const QString& style);
        // void setOpenIconPath(const QString& path);
        // void setOpenIconStyle(const QString& style);
        void setColorSelectIconPath(const QString& path);
        void setColorSelectIconStyle(const QString& style);
        void setToSelectionIconPath(const QString& path);
        void setToSelectionIconStyle(const QString& style);
        void setSearchIconPath(const QString& path);
        void setSearchIconStyle(const QString& style);
        void setSearchActiveIconStyle(const QString& style);

        void setTableIconPath(const QString& path);
        void setTableIconStyle(const QString& style);
        ///@}

        /**
         * Accesses the underlying table model of this GroupingManagerWidget.
         *
         * @returns the GroupingTableModel of this GroupingManagerWidget
         */
        GroupingTableModel* getModel() const
        {
            return mGroupingTableModel;
        }

        /**
         * Get the underlying proxy model that is used to sort and filter the normal model.
         *
         * @return The proxy model.
         */
        GroupingProxyModel* getProxyModel() const
        {
            return mProxyModel;
        }

        /**
         * Enable/Disable the toolbar buttons which require a valid selection
         */
        void setToolbarButtonsEnabled(bool enable);

        /**
         * Enable/Disable the searchbar and update icon accordingly
         */
        void enableSearchbar(bool enable);

        /**
         * Creates a new grouping for successors or predecessors of item.
         *
         * @param maxDepth - maximum recursion depth in view
         * @param succ - if true successors are highlighted, predecessors otherwise
         * @param item - the item to start from. If null recursion starts from first selected item.
         */
        void newGroupingSuccOrPred(int maxDepth, bool succ, const GraphicsItem* item);

        /**
         * Creates groupings dependend on distance to start item.
         *
         * @param maxDepth - maximum recursion depth in view
         * @param succ - if true successors are highlighted, predecessors otherwise
         * @param item - the item to start from. If null recursion starts from first selected item.
         */
        void newGroupingByDistance(int maxDepth, bool succ, const GraphicsItem* item);

    public Q_SLOTS:
        /**
         * Q_SLOT to handle that the last entry of GroupingTableModel was deleted.
         */
        void handleLastEntryDeleted();

        /**
         * Q_SLOT to handle that a new entry was added to the GroupingsTableModel.
         *
         * @param modelIndex - The index of the new entry withing the GroupingsTableModel
         */
        void handleNewEntryAdded(const QModelIndex& modelIndex);

        /**
         * Q_SLOT to handle that the current item of the table views selection model has been changed.
         *
         * @param current - The index of the newly current item.
         * @param previous - The index of the previous current item.
         */
        void handleCurrentChanged(const QModelIndex& current = QModelIndex(), const QModelIndex& previous = QModelIndex());

        /**
         * Adds predecessors of the currently selected gate or module to a new grouping.
         *
         */
        void handleToolboxPredecessor();

        /**
         * Adds successors of the currently selected gate or module to a new grouping.
         *
         */
        void handleToolboxSuccessor();

        /**
         * Performs a BFS with a max-depth of three and creates a new grouping
         * for the predecessors of each depth.
         */
        void handleToolboxPredecessorDistance();

        /**
         * Performs a BFS with a max-depth of three and creates a new grouping
         * for the successors of each depth.
         */
        void handleToolboxSuccessorDistance();

    private Q_SLOTS:
        /**
         * Q_SLOT to toggle the searchbar. If the searchbar was hidden it will be shown. If it was shown it will be hidden.
         */
        void toggleSearchbar();

        /**
         * Q_SLOT to filter the groupings by a string. Used while using the searchbar in the groupings manager widget (CTRL+F).
         *
         * @param text - The filter string
         */
        void filter(const QString& text);

        /**
         * Q_SLOT to update the search icon style. The search icon style indicates wether a filter is applied or not.
         */
        void updateSearchIcon();

        /**
         * Q_SLOT to create a new grouping. Called when the 'Create Grouping'-buttons was clicked.
         */
        void handleCreateGroupingClicked();

        /**
         * Q_SLOT to rename a grouping. Called when the 'Rename Grouping'-buttons was clicked.
         */
        void handleRenameGroupingClicked();

        /**
         * Q_SLOT to show the content of a grouping. Called when the 'Show content'-buttons was clicked.
         */
        void handleShowContentClicked();

        /**
         * Q_SLOT to change the color of a grouping. Called when the color icon was clicked.
         */
        void handleColorSelectClicked();

        /**
         * Q_SLOT to handle that the 'Add to selection'-button was clicked. Used to select the objects of the
         * selected grouping.
         */
        void handleToSelectionClicked();

        /**
         * Q_SLOT to delete a grouping. Called when a grouping was deleted.
         */
        void handleDeleteGroupingClicked();

        /**
         * Q_SLOT to open the right-click-menu within the grouping manager widget.
         *
         * @param point - The click position
         */
        void handleContextMenuRequest(const QPoint& point);

        /**
         * Creates a context menu with four options. A slot that should be connected to the toolbox action.
         */
        void handleToolboxClicked();

        /**
         * Q_SLOT to handle doubleClicked signal. Calls handleRenameGroupingClicked if index.column()==0
         * and handleColorSelectClicked if index.column()==2.
         *
         * @param index - The sender that emitted the change.
         */
        void handleDoubleClicked(const QModelIndex& index);

        void handleDeleteShortcutOnFocusChanged(QWidget *oldWidget, QWidget *newWidget);

    private:
        class ToolboxModuleHash
        {
        public:
            QHash<Gate*, Module*> mHash;
            ToolboxModuleHash(const Node& nd);
        };

        class ToolboxNode
        {
        public:
            QString mName;
            Node mNode;
            ToolboxNode(Endpoint* ep = nullptr, const ToolboxModuleHash* tmh = nullptr);
            ToolboxNode(const GraphicsItem* item);
            std::vector<Net*> inputNets() const;
            std::vector<Net*> outputNets() const;
        };

        QIcon toolboxIcon() const;
        //GraphTabWidget* mTabView;
        QTableView* mGroupingTableView;
        GroupingTableModel* mGroupingTableModel;
        GroupingProxyModel* mProxyModel;
        Searchbar* mSearchbar;

        QAction* mNewGroupingAction;
        QString mNewGroupingIconPath;
        QString mNewGroupingIconStyle;

        QString mToolboxIconPath;
        QString mToolboxIconStyle;

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

        QString mSearchIconPath;
        QString mSearchIconStyle;
        QString mSearchActiveIconStyle;

        QAction* mTableAction;
        QString mTableIconPath;
        QString mTableIconStyle;

        QString mDisabledIconStyle;
        GroupingColorSerializer mColorSerializer;

        QShortcut* mShortCutDeleteItem;

        GroupingTableEntry getCurrentGrouping();
    };
}    // namespace hal

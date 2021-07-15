//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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
    class UserAction;
    class UserActionObject;
    class SettingsItemCheckbox;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Container for all specific details widgets.
     *
     * The SelectionDetailsWidget class is primarily a container used to display on the left side the complete
     * selection as a tree (via a treeview) and on the right side the specific details of a single item chosen
     * from the complete selection (via a stackedwidget containing the specific-details-widgets). This class manages
     * which specific widget to display and provides usefull actions such as adding the current selection to a grouping.
     */
    class SelectionDetailsWidget : public ContentWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString searchIconPath READ searchIconPath WRITE setSearchIconPath)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString searchActiveIconStyle READ searchActiveIconStyle WRITE setSearchActiveIconStyle)
        Q_PROPERTY(QString toGroupingIconPath READ toGroupingIconPath WRITE setToGroupingIconPath)
        Q_PROPERTY(QString toGroupingIconStyle READ toGroupingIconStyle WRITE setToGroupingIconStyle)
        Q_PROPERTY(QString toModuleIconPath READ toModuleIconPath WRITE setToModuleIconPath)
        Q_PROPERTY(QString toModuleIconStyle READ toModuleIconStyle WRITE setToModuleIconStyle)

    public:

        /**
         * The default constructor. Initialization of every child-widget and
         * its connections happens here.
         *
         * @param parent - The widget's parent.
         */
        SelectionDetailsWidget(QWidget* parent = nullptr);

        /**
         * Overrides the ContentWidget's setupToolbar method. Adds its specific actions to the given toolbar
         * so it can be displayed in the ContentFrame.
         *
         * @param toolbar - The toolbar to which the actions are added.
         */
        virtual void setupToolbar(Toolbar* toolbar) override;

        // =====================================================================
        //   Q_PROPERTY functions
        // =====================================================================

        /** @name Q_PROPERTY Functions
         */
        ///@{

        /**
         * Q_PROPERTY READ function for the "disabled"-icon style.
         *
         * @return The "disabled" icon style.
         */
        QString disabledIconStyle() const;

        /**
         * Q_PROPERTY READ function for the "search"-icon path.
         *
         * @return The "search" icon path.
         */
        QString searchIconPath() const;

        /**
         * Q_PROPERTY READ function for the "search"-icon style.
         *
         * @return The "disabled" icon style.
         */
        QString searchIconStyle() const;

        /**
         * Q_PROPERTY READ function for the "search active"-icon style.
         *
         * @return The "search activate" icon style.
         */
        QString searchActiveIconStyle() const;

        /**
         * Q_PROPERTY READ function for the "to grouping"-icon path.
         *
         * @return The "to grouping" icon path.
         */
        QString toGroupingIconPath() const;

        /**
         * Q_PROPERTY READ function for the "to grouping"-icon style.
         *
         * @return The "to grouping" icon style.
         */
        QString toGroupingIconStyle() const;

        /**
         * Q_PROPERTY READ function for the "to module"-icon path.
         *
         * @return The "to module" icon path.
         */
        QString toModuleIconPath() const;

        /**
         * Q_PROPERTY READ function for the "to module"-icon style.
         *
         * @return The "to module" icon style.
         */
        QString toModuleIconStyle() const;

        /**
         * Q_PROPERTY WRITE function for the "disabled"-icon style.
         *
         * @param style - The new style.
         */
        void setDisabledIconStyle(const QString &style);

        /**
         * Q_PROPERTY WRITE function for the "search"-icon path.
         *
         * @param path - The new path.
         */
        void setSearchIconPath(const QString &path);

        /**
         * Q_PROPERTY WRITE function for the "search"-icon style.
         *
         * @param style - The new style.
         */
        void setSearchIconStyle(const QString &style);

        /**
         * Q_PROPERTY WRITE function for the "search active"-icon style.
         *
         * @param style - The new style.
         */
        void setSearchActiveIconStyle(const QString &style);

        /**
         * Q_PROPERTY WRITE function for the "to grouping"-icon path.
         *
         * @param path - The new path.
         */
        void setToGroupingIconPath(const QString &path);

        /**
         * Q_PROPERTY WRITE function for the "to grouping"-icon style.
         *
         * @param style - The new style.
         */
        void setToGroupingIconStyle(const QString &style);
        
        /**
         * Q_PROPERTY WRITE function for the "to module"-icon path.
         *
         * @param path - The new path.
         */
        void setToModuleIconPath(const QString &path);

        /**
         * Q_PROPERTY WRITE function for the "to module"-icon style.
         *
         * @param style - The new style.
         */
        void setToModuleIconStyle(const QString &style);
        ///@}
        
        /**
         * Adds the current selection to the given grouping. The selection is cleared afterwards.
         *
         */
        void selectionToGroupingAction(const QString& existingGrpName = QString());

        /**
         * Returns a UserAction to undo an assignment defined by the given UserActionObject.
         *
         * @param obj - The object containing the assign action.
         * @return The resulting unassign action.
         */
        UserAction* groupingUnassignActionFactory(const UserActionObject& obj) const;

        /**
         * Get the tree view that displays the current selection.
         *
         * @return The tree view.
         */
        SelectionTreeView* selectionTreeView();

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is emitted when the selection within the treeview changes.
         *
         * @param highlight - The items to highlight (that were selected in the view).
         */
        void triggerHighlight(QVector<const SelectionTreeItem*> highlight);

        /**
         * Q_SIGNAL that is emitted when a gate-type item in the treeview is double clicked
         * (or single clicked if it was in the focus to begin with).
         *
         * @param gateId - The id of the clicked gate item.
         */
        void focusGateClicked(u32 gateId);

        /**
         * Q_SIGNAL that is emitted when a net-type item in the treeview is double clicked
         * (or single clicked if it was in the focus to begin with).
         *
         * @param netId - The id of the clicked net item.
         */
        void focusNetClicked(u32 netId);

        /**
         * Q_SIGNAL that is emitted when a module-type item in the treeview is double clicked
         * (or single clicked if it was in the focus to begin with).
         *
         * @param moduleId - The id of the clicked module item.
         */
        void focusModuleClicked(u32 moduleId);

    public Q_SLOTS:

        /**
         * A function that is called when the selection is changed (external). It sets up the widget
         * anew, e.g. populates the treeview and sets up the icons (disabled or enabled style).
         *
         * @param sender - The object that triggered the change.
         */
        void handleSelectionUpdate(void* sender);

        /**
         * A function that is called when the selection of the treeview is changed (single-selection).
         * Responsible to set the specific-details-widget and emits the triggerHighlight signal.
         *
         * @param sti - The selected item.
         */
        void handleTreeSelection(const SelectionTreeItem* sti);

        /**
         * Overriden function of the ContentWidget. Sets up all shortcuts and returns them.
         *
         * @return The List of shortcuts.
         */
        QList<QShortcut*> createShortcuts() override;


    private Q_SLOTS:

        /**
         * Q_SLOT to update the search icon style. The search icon style indicates wether a filter is applied or not.
         */
        void updateSearchIcon();

        /**
         * Opens a context menu and calls, depending on the cosen action, either selectionToNewGrouping()
         * or selectionToExistingGrouping().
         */
        void selectionToGrouping();

        /**
         * Checks all modules if the current selection can be added to that specific module. Thereafter it
         * creates a context menu with all valid modules as options as well as a "New module..." option.
         */
        void selectionToModuleMenu();

        /**
         * Toggles the visibiliy of the searchbar.
         */
        void toggleSearchbar();

        /**
         * Creates a new grouping by calling addDefault() from the GroupingManagerWidget's model and adds
         * the current selection to the grouping.
         */
        void selectionToNewGrouping();

        /**
         * Gets an existing grouping based on the before selected QAction in the ContextMenu created by
         * selectionToGrouping() and adds the current selection to the grouping.
         */
        void selectionToExistingGrouping();

        /**
         * Emits either the focusGateClicked, focusNetClicked or focusModuleClicked signal based on the
         * type of the clicked item.
         *
         * @param sti - The clicked item in the selection-treeview.
         */
        void handleTreeViewItemFocusClicked(const SelectionTreeItem* sti);

    private:

        /**
         * Displays either the GateDetailsWidget, NetDetailsWidget or ModuleDetailsWidget based on the
         * type of the given item.
         *
         * @param sti - The item that is to be displayed.
         */
        void singleSelectionInternal(const SelectionTreeItem* sti);

        /**
         * Adds the current selection to a module selected by id (=actionCode if positive).
         * Create new module an pops up new module dialog if actionCode is negative.
         */
        void selectionToModuleAction(int actionCode);

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

        QAction* mSelectionToGrouping;
        QAction* mSelectionToModule;

        QString mDisabledIconStyle;

        QString mSearchIconPath;
        QString mSearchIconStyle;
        QString mSearchActiveIconStyle;

        QString mToGroupingIconPath;
        QString mToGroupingIconStyle;
        
        QString mToModuleIconPath;
        QString mToModuleIconStyle;
        
        void canMoveToModule(int nodes);
        void enableSearchbar(bool enable);

        static const QString sAddToGrouping;

        static SettingsItemCheckbox* sSettingHideEmpty;
    };
}

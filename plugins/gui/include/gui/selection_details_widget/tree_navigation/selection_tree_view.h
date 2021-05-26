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

#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"

#include <QTreeView>
#include <QWidget>

namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     * @brief Displays the current selection.
     *
     * The SelectionTreeView class displays the entirety of the current selection in a
     * tree-styled fashion (hence the name). It is embedded in the SelectionDetailsWidget.
     */
    class SelectionTreeView : public QTreeView
    {
        Q_OBJECT

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when the current selection within the TreeView is changed
         * (e.g. through a click, this is not used to trigger a "global selection change" but rather
         *  to more specific details of an item within the complete selection).
         *
         * @param sti - The new "selected" item, can be a nullptr if the index was not valid.
         */
        void triggerSelection(const SelectionTreeItem* sti);

        /**
         * Q_SIGNAL that is emitted when an item is double clicked.
         *
         * @param sti - The double clicked item.
         */
        void itemDoubleClicked(const SelectionTreeItem* sti);

        /**
         * Q_SIGNAL that is emitted when the action "Focus item in Graph View" in the context
         * menu (that appears when you right click on an item) is chosen.
         *
         * @param sti - The item that thas right-clicked.
         */
        void focusItemClicked(const SelectionTreeItem* sti);

    public Q_SLOTS:
        /**
         * Tells its intern proxy model to filter its items based on the given string.
         * The first index is automatically selected (if item(s) were found).
         *
         * @param filter_text -The text to filter the model.
         */
        void handleFilterTextChanged(const QString& filter_text);
 
    protected:
        /**
         * Overwritten function to handle a selection change within the view itself. It emits the
         * triggerSelection signal that contains the new item.
         *
         * @param current - The new item.
         * @param previous - The old item.
         */
        virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

        /**
         * Overwritten function that is called when a doubleclick occurs anywhere in the view.
         * If a valid item is doubleclicked the itemDoubleClicked signal is emitted.
         *
         * @param event - The clickevent.
         */
        virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        SelectionTreeView(QWidget* parent = nullptr);

        /**
         * Sets the default width of each column.
         */
        void setDefaultColumnWidth();

        /**
         * Triggers the underlying model to update its data to the new selection and therefore
         * filling the view if mVisible is set to True. If mVisible is set to False, the view's
         * selection model is cleared and the view itself hides.
         *
         * @param mVisible - The bool to determine the described behaviour.
         */
        void populate(bool mVisible);

        /**
         * Converts a given modelIntex to the item it represents.
         *
         * @param index - The index to convert.
         * @return The item that is represented. Returns a nullptr if the index is invalid or the conversion fails.
         */
        SelectionTreeItem* itemFromIndex(const QModelIndex& index = QModelIndex()) const;

        /**
         * Get the view's proxy model for the SelectionTreeModel.
         *
         * @return The proxy model.
         */
        SelectionTreeProxyModel* proxyModel();

    private Q_SLOTS:
        void handleCustomContextMenuRequested(const QPoint& point);
        void handleIsolationViewAction(const SelectionTreeItem* sti);

    private:
        SelectionTreeModel* mSelectionTreeModel;
        SelectionTreeProxyModel* mSelectionTreeProxyModel;
    };
}    // namespace hal

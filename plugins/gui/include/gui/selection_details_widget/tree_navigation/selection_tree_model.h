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

#include "hal_core/defines.h"
#include "hal_core/netlist/event_handler.h"
#include "gui/gui_utils/sort.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QFont>
#include <QIcon>
#include <QVariant>

namespace hal
{
    class SelectionTreeItem;
    class SelectionTreeItemModule;
    class SelectionTreeItemRoot;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief A model that contains the current selection.
     *
     * A model that manages the current selection in a tree-styled fashion.
     * Its most important function is fetchSelection that automatically updates
     * the model's internal data.
     */
    class SelectionTreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The model's parent.
         */
        SelectionTreeModel(QObject* parent = nullptr);

        /**
          * The destructor.
          */
        ~SelectionTreeModel();

        /** @name Overwritten model functions.
          */
        ///@{

        //information access
        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        QVariant data(const QModelIndex& index, int role) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        QModelIndex parent(const QModelIndex& index) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        ///@}

        /**
         * Updates its internal data. If hasEntries is set to True, the current selection is fetched
         * from the selectionRelay. Otherwise the model is simply cleared.
         *
         * @param hasEntries - Decides wether the current selection is fetched.
         */
        void fetchSelection(bool hasEntries);

        /**
         * Helper functions to convert between an item and its corresponding index.
         *
         * @param item - The item from which to get the index.
         * @return The index that holds the item.
         */
        QModelIndex indexFromItem(SelectionTreeItem* item) const;

        /**
         * A recursive function to get the suppressed items that are not matched by the given regular
         * expression (and therefore should not be displayed). The ids of theese items are then stored
         * in the appropriate lists.
         *
         * @param modIds - The list that holds the ids of the suppressed modules.
         * @param gatIds - The list that holds the ids of the suppressed gates.
         * @param netIds - The list that holds the ids of the suppressed nets.
         * @param regex - The regex to match the items against.
         */
        void suppressedByFilter(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                const QRegularExpression& regex) const;

        static const int sNameColumn = 0;
        static const int sIdColumn   = 1;
        static const int sTypeColumn = 2;
        static const int sMaxColumn  = 3;

    public Q_SLOTS:
        /**
         * Q_SLOT to handle the change of a gate and emits the dataChanged signal. This
         * function is connected to the gateNameChanged signal.
         *
         * @param gate - The gate whose information changed.
         */
        void handleGateItemChanged(Gate* gate);

        /**
         * Q_SLOT to handle the change of a module and emits the dataChanged signal. This
         * function is connected to the moduleNameChanged and moduleTypeChanged signal.
         *
         * @param module - The module whose information changed.
         */
        void handleModuleItemChanged(Module* module);

    private:

        void moduleRecursion(SelectionTreeItemModule* modItem);
        bool doNotDisturb(const QModelIndex& inx = QModelIndex()) const;
        SelectionTreeItem* itemFromIndex(const QModelIndex& index) const;
        SelectionTreeItem* getItem(SelectionTreeItem *parentItem,
                                   const SelectionTreeItem& needle) const;

        SelectionTreeItemRoot* mRootItem;

        /// avoid calls while model is under reconstruction
        int mDoNotDisturb;
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Deletes the current SelectionTreeModel.
     *
     * A utility class to delete the current model. The current root item of the
     * model is given to the disposer and deleted when dispose is called.
     */
    class SelectionTreeModelDisposer : public QObject
    {
        Q_OBJECT
    public:

        /**
         * The constructor.
         *
         * @param stim - The root item to "wrap".
         * @param parent - The disposer's parent.
         */
        SelectionTreeModelDisposer(SelectionTreeItemRoot* stim, QObject* parent=nullptr);

    public Q_SLOTS:

        /**
         * Deletes its root item.
         */
        void dispose();

    private:
        SelectionTreeItemRoot* mRootItem;
    };


}

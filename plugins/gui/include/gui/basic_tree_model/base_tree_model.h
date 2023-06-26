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

#include <QAbstractItemModel>
#include "base_tree_item.h"

namespace hal
{
    /**
     * @ingroup gui
     * @brief The BaseTreeModel implements generic standard functions of a tree model.
     *
     * Explain usage here:
     */
    class BaseTreeModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        /**
         * The constructor.
         *
         * @param parent - The model's parent.
         */
        BaseTreeModel(QObject* parent = nullptr);

        /** @name Overwritten model functions
         */
        ///@{

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual QModelIndex parent(const QModelIndex &index) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        ///@}


        /**
         * Sets the content the tree then represents. An already complete tree (or rather all their
         * first-level items) must be provided.
         *
         * @param firstLevelItems - All items that will be appended to this model's root item.
         */
        void setContent(QList<BaseTreeItem*> firstLevelItems);

        /**
         * Resets the model (deletes the tree).
         */
        virtual void clear();

        /**
         * This function must be called first (as of now) in the constructor of the derived
         * classed to initialize
         * @param labels
         */
        void setHeaderLabels(const QStringList& label);

        // important converter methods
        /**
         * Helper method to convert between the item and its index.
         *
         * @param item - The item from which to get the index.
         * @return The index.
         */
        QModelIndex getIndexFromItem(BaseTreeItem* item) const;

        /**
         * Helper method to convert between the index and its item.
         *
         * @param index - The index to convert.
         * @return The internal item.
         */
        BaseTreeItem* getItemFromIndex(QModelIndex index) const;

        /**
         * Convenient function to get the root item to which the tree is appended
         * (the root item is not shown/displayed).
         *
         * @return The root item.
         */
        BaseTreeItem* getRootItem() const;

    protected:
        BaseTreeItem* mRootItem;
    };

}

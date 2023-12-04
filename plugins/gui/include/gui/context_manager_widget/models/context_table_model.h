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

#include "hal_core/defines.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/basic_tree_model/base_tree_model.h"



namespace hal
{

    class ContextDirectory
    {
        private:
            u32 mId;
            QString mName;

        public:
            ContextDirectory(u32 id, QString name):mId(id), mName(name){}

    };

    class ContextTreeItem : public BaseTreeItem
    {

        private:
            GraphContext* mContext;
            ContextDirectory* mDirectory;
        public:

            ContextTreeItem(GraphContext* context);
            QVariant getData(int column) const override;
            void setData(QList<QVariant> data) override;
            void setDataAtIndex(int index, QVariant& data) override;
            void appendData(QVariant data) override;
            int getColumnCount() const override;
            int row() const;
            bool isDirectory() const;
    };

    /**
     * @ingroup utility_widgets-context
     * @brief Base model for the ContextManagerWidget to manage GraphContext%s.
     *
     * The ContextTableModel implements a standard table model to manage the GraphContext%s
     * that are created/deleted/modified by the user. It provides the neccessary interface for
     * the ContextManagerWidget to store and display the data. For specific information on how to
     * implement a table model, refer to qt's QAbstractTableModel class and its examples.
     */
    class ContextTreeModel : public BaseTreeModel
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        ContextTreeModel(QObject* parent = nullptr);

        /** @name Overwritten model functions
         */
        ///@{
        //int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& inddex, int role = Qt::DisplayRole) const override;
        ///@}

        /**
         * Adds a given GraphContext to the model.
         *
         * @param context - The context to add.
         * @param parent - The Parent of the context.
         */
        void addContext(GraphContext* context, BaseTreeItem* parent = nullptr);

        /**
         * Removes a given GraphContext from the model.
         *
         * @param context - The context to remove.
         */
        void removeContext(GraphContext* context);

        /**
         * Get the GraphContext from a given index.
         *
         * @param index - The index of the requested GraphContext.
         * @return The requested GraphContext.
         */
        GraphContext* getContext(const QModelIndex& index) const;

        /**
         * Converts a given GraphContext into its corresponding index.
         *
         * @param context - The context to convert.
         * @return The resulting index.
         */
        QModelIndex getIndex(GraphContext* context) const;

        /**
         * Returns the index where the specified ContextTreeitem can be found.
         *
         * @param item - The ContextTreeitem to search for in the item model
         * @returns the model index of the specified ContextTreeitem
         */
        //QModelIndex getIndex(const BaseTreeItem * const item) const;

        /**
         * Resets the model (removes all GraphContext%s).
         */
        void clear();

        /**
         * Get all GraphContext%s of the model.
         *
         * @return A vector of all GraphContext%s.
         */
        const QVector<GraphContext*>& list() const;
    private Q_SLOTS:
        void handleDataChanged();

    private:
        ContextTreeItem *mCurrentDirectory;
        std::map<GraphContext *, ContextTreeItem *> mContextMap;
    };
}    // namespace hal

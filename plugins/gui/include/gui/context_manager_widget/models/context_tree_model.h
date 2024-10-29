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

#include <QMimeData>

namespace hal
{

    class ContextDirectory
    {
        private:
            u32 mId;
            QString mName;
            u32 mParentId;

        public:
            ContextDirectory(QString name, u32 parentId, u32 id):mName(name){
                mId = id;
                mParentId = parentId;
            }

            /**
             * Writes the directory to a given json object.
             *
             * @param json - The object to write to.
             */
            void writeToFile(QJsonObject& json);

            QString name() const { return mName; }
            u32 id() const { return mId; }
            void setId(u32 id_) { mId = id_; }
            void setName(QString name_) { mName = name_; }
    };

    class ContextTreeItem : public BaseTreeItem
    {
        private:
            GraphContext* mContext;
            ContextDirectory* mDirectory;
        public:

            ContextTreeItem(GraphContext* context);
            ContextTreeItem(ContextDirectory* directory);
            QVariant getData(int column) const override;
            void setData(QList<QVariant> data) override;
            void setDataAtIndex(int index, QVariant& data) override;
            void appendData(QVariant data) override;
            int getColumnCount() const override;
            int row() const;
            bool isDirectory() const;
            bool isContext() const;
            u32 getId() const;
            QString getName() const;
            QDateTime getTimestamp() const;
            GraphContext* context() const;
            ContextDirectory* directory() const;
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
         * Returns the ContextTreeItem of a directory specified by an id.
         * 
         * @param directoryId - The id of the directory whose ContextTreeItem should be returned.
         * 
         * @return The ContextTreeItem for the directory specified by the id.
         */
        BaseTreeItem* getDirectory(u32 directoryId) const;

        /**
         * Returns the ContextTreeItem of a context specified by an id.
         * 
         * @param contextId - The id of the context whose ContextTreeItem should be returned.
         * 
         * @return The ContextTreeItem for the context specified by the id.
         */
        BaseTreeItem* getContext(u32 contextId) const;

        /**
         * Adds a directory to the model.
         *
         * @param name - The name to the directory.
         * @param parentItem - The parent of the directory.
         * @param id - The id of the directory.
         */
        ContextDirectory* addDirectory(QString name, BaseTreeItem* parentItem, u32 id);

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
         * Removes a given contextDirectory from the model.
         *
         * @param directory - The directory to remove.
         */
        void removeDirectory(ContextDirectory* directory);

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
        QModelIndex getIndexFromContext(GraphContext* context) const;

        /**
         * Resets the model (removes all GraphContext%s).
         */
        void clear() override;

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QStringList mimeTypes() const override;

        /**
         * Get all GraphContext%s of the model.
         *
         * @return A vector of all GraphContext%s.
         */
        const QVector<GraphContext*>& list();

        /**
         * Get all ContextDirectories of the model.
         *
         * @return A vector of all ContextDirectories.
         */
        const QVector<ContextDirectory*>& directoryList();

        /**
         * Sets the CurrentDirectory.
         *
         * @param ContextTreeItem - The Currently focused Item.
         */
        void setCurrentDirectory(ContextTreeItem* currentItem);

        /**
         * Gets the CurrentDirectory.
         *
         * @return current directory.
         */
        ContextTreeItem* getCurrentDirectory();

        /**
         * Removes a tree item and inserts it under a new parent into a specific row.
         * 
         * @param itemToMove - The tree item to be moved.
         * @param newParent - The parent item, under which itemToMove is placed.
         * @param row - The row in newParent, where itemToMove is inserted. 
         *              If -1, then itemToMove is instead just appended to newParent.
         * @return True, if the operation succeded. False, if not.
         */
        bool moveItem(ContextTreeItem* itemToMove, BaseTreeItem* newParent, int row = -1);

        /**
         * Returns the ids of all direct child directories of a given parent directory.
         * 
         * @param directoyId - The id of the parent directory.
         * @return List of IDs of all directories, that are ordered directly under the parent directory.
         */
        std::vector<u32> getChildDirectoriesOf(u32 directoryId);

        /**
         * Returns the ids of all direct child contexts of a given parent directory.
         * 
         * @param directoyId - The id of the parent directory.
         * @return List of IDs of all contexts, that are ordered directly under the parent directory.
         */
        std::vector<u32> getChildContextsOf(u32 directoryId);

    Q_SIGNALS:
        void directoryCreatedSignal(ContextTreeItem* item);

    private:
        BaseTreeItem* getItemInternal(BaseTreeItem* parentItem, u32 id, bool isDirectory) const;
        ContextTreeItem *mCurrentDirectory;
        std::map<GraphContext *, ContextTreeItem *> mContextMap;
        QVector<GraphContext*> mContextList;
        QVector<ContextDirectory*> mDirectoryList;

        void dumpRecursion(ContextTreeItem* parent = nullptr, int level = 0) const;
    };
}    // namespace hal

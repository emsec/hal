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
#include "gui/basic_tree_model/base_tree_model.h"


#include <QColor>
#include <QList>
#include <QString>
#include <QVariant>

namespace hal
{
    class ModuleModel;

    /**
     * @ingroup gui
     * @brief An item in the ModuleModel.
     *
     * The ModuleItem is one item in the ModuleModel item model. It represents either a module, a gate or a net of the netlist.
     */
    class ModuleItem : public BaseTreeItem
    {
    public:
        /**
         * The possible types that a ModuleItem in the ModuleModel can have.
         */
        enum class TreeItemType {Module, Gate, Net};

        /**
         * Sets the data for the columns name and type. 
         * Column 2 (type) can be only set, if this item is a module.
         *
         * @param data - Each entry in the list represents one column. 
         *               The second column (id) is ignored.
         */
        void setData(QList<QVariant> data) override;
        /**
         * Sets the data for a specified column. Column 2 (type) can only be set, if this item is a module.
         *
         * @param index - The column to set the new data. Either 0 (name) or 2(type). Other columns will be ignored.
         * @param data - The new column data.
         */
        void setDataAtIndex(int index, QVariant& data) override;
        /**
         * Unused dummy function overwritten from parent class.
         */
        void appendData(QVariant data) override;
        /**
         * Get the number of currently stored column data.
         *
         * @return 3
         */
        int getColumnCount() const override;

        /**
         * Constructor.
         *
         * @param id - The id of the netlist item this ModuleItem represents
         * @param type - The type of the netlist item
         * @param model - The parent model where this item is added to.
         */
        ModuleItem(const u32 id, const TreeItemType type, ModuleModel* model);

        /**
         * Destructor.
         *
         * Must remove item from ModuleModel map as well.
         */
        virtual ~ModuleItem();

        /**
         * Given a set of ModuleItems (in a map [id]->[ModuleItem]) this function adds each ModuleItem of this set as
         * a new children if its underlying module is a submodule (child) of the underlying module of this ModuleItem.
         *
         * @param moduleMap - A map [id]->[ModuleItem] of children candidates
         */
        void appendExistingChildIfAny(const QMap<u32,ModuleItem*>& moduleMap);

        /**
         * Gets the data of this item model item i.e. the name of this ModuleItem if column=1.
         *
         * @param column - The column to get the data for
         * @returns the data in the specified column of this ModuleItem
         */
        QVariant getData(int column) const override;

        /**
         * Gets the name of the netlist item this ModuleItem represents.
         *
         * @returns the netlist items name
         */
        QString name() const;

        /**
         * Gets the id of the netlist item this ModuleItem represents.
         *
         * @returns the module id
         */
        u32 id() const;

        /**
         * Checks if this ModuleItem is currently highlighted.
         *
         * @returns <b>true</b> if this ModuleItem is currently highlighted.
         */
        bool highlighted() const;

        /**
         * Checks if this ModuleItem is direct child to mRootItem
         *
         * @return
         */
        bool isToplevelItem() const;

        /**
         * Gets the type of the netlist item this ModuleItem represents.
         *
         * @returns the ModuleItem type
         */
        TreeItemType getType() const;

        /**
         * Sets the name of this ModuleItem (not the underlying module).
         *
         * @param name - The new name
         */
        void setName(const QString& name);

        /**
         * Set the module type name (gate types are immutable)
         * @param moduleType
         */
        void setModuleType(const QString& moduleType);

        /**
         * Marks/Unmarks this ModuleItem as highlighted.
         *
         * @param highlighted - <b>true</b> if the ModuleItem should be marked as highlighted.
         *                      <b>false</b> if the ModuleItem should be marked as NOT highlighted.
         */
        void setHighlighted(const bool highlighted);

    private:
        u32 mId;
        TreeItemType mItemType;
        QString mName;
        QString mModuleType;

        bool mHighlighted;
        ModuleModel* mModuleModel; // reference to parent model
    };
}

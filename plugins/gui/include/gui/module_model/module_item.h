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

#include <QColor>
#include <QList>
#include <QString>
#include <QVariant>

namespace hal
{
    /**
     * @ingroup gui
     * @brief An item in the ModuleModel.
     *
     * The ModuleItem is one item in the ModuleModel item model. It represents one module of the netlist.
     */
    class ModuleItem
    {
    public:
        /**
         * Constructor.
         *
         * @param id - The id of the module this item represents
         */
        ModuleItem(const u32 id);

        /**
         * Appends a child ModuleItem to this ModuleItem.
         *
         * @param row - The index of the childs of this ModuleItem the new child should be moved to
         * @param child - The new child to be inserted
         */
        void insertChild(int row, ModuleItem* child);

        /**
         * Removes a child ModuleItem from this ModuleItem.
         *
         * @param child - The child to remove
         */
        void removeChild(ModuleItem* child);

        /**
         * Inserts a child ModuleItem at the end of the children list of this ModuleItem.
         *
         * @param child - The child to be appended
         */
        void appendChild(ModuleItem* child);

        /**
         * Given a set of ModuleItems (in a map [id]->[ModuleItem]) this function adds each ModuleItem of this set as
         * a new children if its underlying module is a submodule (child) of the underlying module of this ModuleItem.
         *
         * @param moduleMap - A map [id]->[ModuleItem] of children candidates
         */
        void appendExistingChildIfAny(const QMap<u32,ModuleItem*>& moduleMap);

        /**
         * Inserts a child ModuleItem at the beginning of the children list of this ModuleItem.
         *
         * @param child - The child to be prepended
         */
        void prependChild(ModuleItem* child);

        /**
         * Gets the parent ModuleItem of this ModuleItem.
         *
         * @returns the parent ModuleItem
         */
        ModuleItem* parent();

        /**
         * Get the child ModuleItem at a certain position in the children list of this ModuleItem.
         *
         * @param row - The position in the children list of this ModuleItem
         * @returns then children ModuleItem at the specified position in the children list
         */
        ModuleItem* child(int row);

        /**
         * Gets the parent ModuleItem of this ModuleItem.
         *
         * @returns the parent ModuleItem. Returns a constant ModuleItem pointer
         */
        const ModuleItem* constParent() const;

        /**
         * Get the child ModuleItem at a certain position in the children list of this ModuleItem.
         *
         * @param row - The position in the children list of this ModuleItem
         * @returns then children ModuleItem at the specified position in the children list.
         *          Returns a constant ModuleItem pointer
         */
        const ModuleItem* constChild(int row) const;

        /**
         * Gets the current amount of children of this ModuleItem
         *
         * @returns the amount of children
         */
        int childCount() const;

        /**
         * Gets the data of this item model item i.e. the name of this ModuleItem if column=1.
         *
         * @param column - The column to get the data for
         * @returns the data in the specified column of this ModuleItem
         */
        QVariant data(int column) const;

        /**
         * Gets the index of this ModuleItem in the list of children ModuleItems of its parent.
         *
         * @returns the index in the parents ModuleItem children list
         */
        int row() const;

        /**
         * Gets the name of the module this ModuleItem represents.
         *
         * @returns the modules name
         */
        QString name() const;

        /**
         * Gets the id of the module this ModuleItem represents.
         *
         * @returns the module id
         */
        u32 id() const;

        /**
         * Gets the color of the module this ModuleItem represents.
         *
         * @returns the module color
         */
        QColor color() const;

        /**
         * Checks if this ModuleItem is currently highlighted.
         *
         * @returns <b>true</b> iff this ModuleItem is currently highlighted.
         */
        bool highlighted() const;

        /**
         * Sets the parent ModuleItem of this ModuleItem.
         *
         * @param parent - The new parent
         */
        void setParent(ModuleItem* parent);

        /**
         * Sets the name of this ModuleItem (not the underlying module).
         *
         * @param name - The new name
         */
        void set_name(const QString& name);

        /**
         * Sets the color of the module this ModuleItem represents.
         *
         * @param color - The new color
         */
        void setColor(const QColor& color);

        /**
         * Marks/Unmarks this ModuleItem as highlighted.
         *
         * @param highlighted - <b>true</b> if the ModuleItem should be marked as highlighted.
         *                      <b>false</b> if the ModuleItem should be marked as NOT highlighted.
         */
        void setHighlighted(const bool highlighted);

    private:
        ModuleItem* mParent;
        QList<ModuleItem*> mChildItems;

        u32 mId;
        QString mName;

        QColor mColor;
        bool mHighlighted;
    };
}

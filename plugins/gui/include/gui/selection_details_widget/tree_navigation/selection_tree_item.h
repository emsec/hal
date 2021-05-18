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
#include <QList>
#include <QVariant>
#include <QList>
#include <QIcon>
#include <QSet>
#include <QRegularExpression>

namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     * @brief Basic item for the SelectionTreeModel.
     *
     * The basic tree item class for the SelectionTreeModel. It is the base class for every other
     * subitem (except for SelectionTreeItemRoot) as it provides all commonly shared and necessary
     * functions for the model and related events.
     */
    class SelectionTreeItem
    {
    public:

        /**
         * An enum that defines all possible types a tree item can take with the exception
         * of the NullItem and MaxItem type. These types are convenient type used in the model
         * and the graphics scene (NullItem is used for the root item of the model). The type
         * be accessed via the itemType() method.
         */
        enum TreeItemType
        {
            NullItem, ModuleItem, GateItem, NetItem, MaxItem
        };

        /**
         * The constructor.
         *
         * @param t - The type based on the netlist item type it represents.
         * @param id_ The id of the item. It usually is the same as the underlying netlist item.
         */
        SelectionTreeItem(TreeItemType t = NullItem, u32 id_ = 0);

        /**
         * The destructor.
         */
        virtual ~SelectionTreeItem();

        //information access
        /**
         * Get the item's type.
         *
         * @return The type.
         */
        TreeItemType itemType() const;

        /**
         * Get the item's id.
         *
         * @return The id.
         */
        u32 id() const;

        /**
         * Get the item's parent (the direct ancestor in the tree).
         *
         * @return The parent.
         */
        SelectionTreeItem* parent() const;

        /**
         * Sets the parent (direct ancestor in the tree) of the item.
         *
         * @param p - The parent.
         */
        void setParent(SelectionTreeItem* p);

        /**
         * Get the number of child item (the items to which this item is the direct ancestor in the tree).
         *
         * @return The number of children.
         */
        virtual int childCount() const;

        /**
         * Get the child from the given row.
         *
         * @param row - The row for which the child is requested.
         * @return The child for the given row. If the row is not in the childCount range, a nullptr is returned.
         */
        virtual SelectionTreeItem* child(int row) const;

        /**
         * Get the data for the specified column.
         *
         * @param column - The column for which the data is requested.
         * @return The data. If the column > 2, an empty QVariant is returned.
         */
        virtual QVariant data(int column) const;

        /**
         * Get the name of the item. The name is usually the name of the underlying netlist item.
         * Must be implemented in the subclasse(s).
         *
         * @return The item's name.
         */
        virtual QVariant name() const = 0;

        /**
         * Get the icon for either the module, gate, or net item. Must be implemented by
         * the specific subclass.
         *
         * @return The subclasse's specific icon.
         */
        virtual const QIcon& icon() const = 0;


        /**
         * Returns the appropriate type of either a gate or module (not a net).
         * Must be implemented in the subclass.
         *
         * @return The name of the type.
         */
        virtual QVariant boxType() const;

        /**
         * Matches the given regex against the name, id, and in case if the the item is a gate type,
         * the gate type. If the item is a module type, the children are also matched against.
         *
         * @param regex -  The regex to match against the item.
         * @return True if a match in either of the categories was found. False otherwise.
         */
        virtual bool match(const QRegularExpression& regex) const;

        /**
         * A function that fills the modIds, gateIds, and netIds lists with the respective ids if the
         * called selection-tree-item subclass does not match the given regex (and in thus suppressed,
         * meaning should not be displayed). Must be implemented by the specific subclass since a module-type
         * item must call this function of its children. A gate- or net-type item simply adds itself to the list
         * (or not).
         *
         * @param modIds - The list that contains the suppresed module-type items at the end.
         * @param gatIds - The list that contains the suppressed gate-type items at the end.
         * @param netIds - The list that contains the suppressed net-type items at the end.
         * @param regex - The regular expression to match the items against.
         */
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const = 0;
        bool isEqual(const SelectionTreeItem* sti) const;
    protected:
        TreeItemType mItemType;
        u32 mId;
        SelectionTreeItem* mParent;
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Tree item that represents a module.
     *
     * A subclass of the basic tree item class that represents a module of the netlist.
     * It adds new hierarchical functionality since the module type item is the only item that
     * can have children (and thus building the tree structure).
     */
    class SelectionTreeItemModule : public SelectionTreeItem
    {
    public:

        /**
         * The constructor.
         *
         * @param id_ - The item's id. It is the same id as the module it represents.
         */
        SelectionTreeItemModule(u32 id_);

        /**
          * The destructor.
          */
        ~SelectionTreeItemModule();

        /**
         * Get the number of child item (the items to which this item is the direct ancestor in the tree).
         *
         * @return The number of children.
         */
        virtual int childCount() const;

        /**
         * Get the child from the given row.
         *
         * @param row - The row for which the child is requested.
         * @return The child for the given row. If the row is not in the childCount range, a nullptr is returned.
         */
        virtual SelectionTreeItem* child(int row) const;

        /**
         * Get the name of the item. The name is usually the name of the underlying netlist item.
         *
         * @return The item's name.
         */
        virtual QVariant name() const;

        /**
         * Get the icon for a module type item.
         *
         * @return The module specific icon.
         */
        virtual const QIcon& icon() const;

        /**
         * Returns the name of the module's type.
         *
         * @return The name of the module's type.
         */
        virtual QVariant boxType() const;

        /**
         * Calls the match() function of its children and matches the regex against its id and name.
         *
         * @param regex -  The regex to match against.
         * @return True if a match in either of the categories was found or if a child item returns True. False otherwise.
         */
        virtual bool     match(const QRegularExpression& regex) const;

        /**
         * Matches itself against the given regex. If no mbrief boxTypeatch was found or the module item is the root of
         * the model, the item inserts itself in the modIds list. This function is then invoked on all of its
         * children.
         *
         * @param modIds - The list that contains the suppresed module-type items at the end.
         * @param gatIds - The list that contains the suppressed gate-type items at the end.
         * @param netIds - The list that contains the suppressed net-type items at the end.
         * @param regex - The regular expression to match the items against.
         */
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const;

        /**
         * Appends the given tree item to its children.
         *
         * @param cld - The item to append.
         */
        void addChild(SelectionTreeItem* cld);

        /**
         * Checks if the module is the root item of the model.
         *
         * @return True if the item is the root item. False otherwise.
         */
        bool isRoot() const;
    protected:
        bool mIsRoot;
        QList<SelectionTreeItem*> mChildItem;
    private:
        static QIcon* sIconInstance;
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Tree item that represent the root.
     *
     * A special case of the module type tree item. It has the fixed id 0.
     */
    class SelectionTreeItemRoot : public SelectionTreeItemModule
    {
    public :
        /**
         * The constructor. The fixed id 0 is set here.
         */
        SelectionTreeItemRoot();
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Tree item that represents a gate.
     *
     * A subclass of the basic tree item class that represents a gate of the netlist.
     * It implements the necessary commonly shared inherited functions as well as the
     * gate specific function gateType().
     */
    class SelectionTreeItemGate : public SelectionTreeItem
    {
    public:

        /**
         * The constructor.
         *
         * @param id_ - The item's id. It is the same id as the gate it represents.
         */
        SelectionTreeItemGate(u32 id_);

        /**
         * Get the name of the item. The name is usually the name of the underlying netlist item.
         *
         * @return The item's name.
         */
        virtual QVariant name() const;

        /**
         * Get the icon for a gate type item.
         *
         * @return The gate specific icon.
         */
        virtual const QIcon& icon() const;

        /**
         * Matches itself against the given regex. If no match was found it appends itself (its id)
         * to the gatIds list.
         *
         * @param modIds - The list that contains the suppresed module-type items at the end.
         * @param gatIds - The list that contains the suppressed gate-type items at the end.
         * @param netIds - The list that contains the suppressed net-type items at the end.
         * @param regex - The regular expression to match the items against.
         */
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const;

        /**
         * Get the name of the gate's type (e.g. LUT5 or FF).
         *
         * @return The gate type (QString).
         */
        virtual QVariant boxType() const;
    private:
        static QIcon* sIconInstance;
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Tree item that represents a net.
     *
     * A subclass of the basic tree item class that represents a net of the netlist.
     * It implements the necessary commonly shared inherited functions.
     */
    class SelectionTreeItemNet : public SelectionTreeItem
    {
    public:

        /**
         * The constructor.
         *
         * @param id_ - The item's id. It is the same id as the net it represents.
         */
        SelectionTreeItemNet(u32 id_);

        /**
         * Get the name of the item. The name is usually the name of the underlying netlist item.
         *
         * @return The item's name.
         */
        virtual QVariant name() const;

        /**
         * Get the icon for a net type item.
         *
         * @return The net net icon.
         */
        virtual const QIcon& icon() const;

        /**
         * Matches itself against the given regex. If no match was found it appends itself (its id)
         * to the netIds list.
         *
         * @param modIds - The list that contains the suppresed module-type items at the end.
         * @param gatIds - The list that contains the suppressed gate-type items at the end.
         * @param netIds - The list that contains the suppressed net-type items at the end.
         * @param regex - The regular expression to match the items against.
         */
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const;
    private:
        static QIcon* sIconInstance;
    };

}

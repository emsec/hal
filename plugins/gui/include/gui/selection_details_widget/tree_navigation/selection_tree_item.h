//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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
    class SelectionTreeItem
    {
    public:
        enum TreeItemType
        {
            NullItem, ModuleItem, GateItem, NetItem, MaxItem
        };

        SelectionTreeItem(TreeItemType t = NullItem, u32 id_ = 0);
        virtual ~SelectionTreeItem();

        //information access
        TreeItemType itemType() const;
        u32 id() const;
        SelectionTreeItem* parent() const;
        void setParent(SelectionTreeItem* p);
        virtual int childCount() const;
        virtual SelectionTreeItem* child(int row) const;
        virtual QVariant data(int column) const;
        virtual QVariant name() const = 0; // implemented in subclass
        virtual QVariant gateType() const;
        virtual QIcon    icon() const = 0;
        virtual bool     match(const QRegularExpression& regex) const;

        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const = 0;
    protected:
        TreeItemType mItemType;
        u32 mId;
        SelectionTreeItem* mParent;
    };

    class SelectionTreeItemModule : public SelectionTreeItem
    {
    public:
        SelectionTreeItemModule(u32 id_);
        ~SelectionTreeItemModule();
        virtual int childCount() const;
        virtual SelectionTreeItem* child(int row) const;
        virtual QVariant name() const;
        virtual QIcon    icon() const;
        virtual bool     match(const QRegularExpression& regex) const;
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const;
        void addChild(SelectionTreeItem* cld);
        bool isRoot() const;
    protected:
        bool mIsRoot;
        QList<SelectionTreeItem*> mChildItem;
    };

    class SelectionTreeItemRoot : public SelectionTreeItemModule
    {
    public :
        SelectionTreeItemRoot();
    };

    class SelectionTreeItemGate : public SelectionTreeItem
    {
    public:
        SelectionTreeItemGate(u32 id_);
        virtual QVariant name() const;
        virtual QIcon    icon() const;
        virtual QVariant gateType() const;
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const;
    };

    class SelectionTreeItemNet : public SelectionTreeItem
    {
    public:
        SelectionTreeItemNet(u32 id_);
        virtual QVariant name() const;
        virtual QIcon    icon() const;
        virtual void suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                 const QRegularExpression& regex) const;
    };

}

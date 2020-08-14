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

#include "def.h"
#include "gui/gui_utils/sort.h"
#include "netlist/module.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <set>

namespace hal
{
    class ModuleItem;

    class ModuleModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit ModuleModel(QObject* parent = nullptr);

        // PURE VIRTUAL
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role) const override;

        // VIRTUAL
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        ModuleItem* get_item(const QModelIndex& index) const;
        QModelIndex get_index(const ModuleItem* const item) const;
        ModuleItem* get_item(const u32 module_id) const;

        void init();
        void clear();

        void add_module(const u32 id, const u32 parent_module);
        void add_recursively(std::set<std::shared_ptr<Module>> modules);
        void remove_module(const u32 id);
        void update_module(const u32 id);

        bool is_modifying();

    private:
        ModuleItem* m_top_ModuleItem;

        QMap<u32, ModuleItem*> m_ModuleItems;

        bool m_is_modifying;
    };
}    // namespace hal

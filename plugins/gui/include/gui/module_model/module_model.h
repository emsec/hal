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
#include "gui/gui_utils/sort.h"
#include "hal_core/netlist/module.h"
#include "gui/module_model/module_item.h"


#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <set>
#include <array>

namespace hal
{

    /**
     * @ingroup gui
     * @brief Represents the netlist module's hierarchy.
     *
     * The ModuleModel is the item model that represents the modules and their hierarchy in the netlist.
     */
    class ModuleModel : public BaseTreeModel
    {
        Q_OBJECT

    public:
        /**
         * Constructor. <br>
         * Since the netlist is not necessarily loaded when this class is instantiated, the model won't be filled with
         * data until the init function is called. The constructor is an empty one.
         *
         * @param parent - The parent object.
         */
        explicit ModuleModel(QObject* parent = nullptr);

        // === Pure Virtual ===
        /**
         * Returns the index of the item in the model specified by the given row, column and parent index.
         *
         * @param row - The row of the item
         * @param column - The column of the item
         * @param parent - the index of the parent of the item
         * @returns the index at the specified position
         */
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

        /**
         * Returns the parent of the model item with the given index. If the item has no parent (i.e. index is
         * invalid or module is the top module), and invalid QModelIndex is returned.
         *
         * @param index - The index to find the parent for
         * @returns the model index of the parent
         */
        QModelIndex parent(const QModelIndex& index) const override;

        /**
         * Returns the number of rows under the given parent (i.e. the number of children of the parent).
         *
         * @param parent - The model index of the parent
         * @returns the number of rows under the given parent
         */
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        /**
         * Returns the number of columns for the children of the given parent. Since the module model only contains
         * one column this function returns always 1.
         *
         * @param parent - The model index of the parent
         * @returns the number of columns for the children of the given parent. Always 1.
         */
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        /**
         * Returns the data stored under the given role for the item referred to by the index.
         *
         * @param index - The model index
         * @param role - The role the model is accessed with
         * @returns the data stored at index under the specified role
         */
        QVariant data(const QModelIndex& index, int role) const override;

        // === Virtual ===
        /**
         * Returns the item flags for the given index.
         *
         * @param index - The model index to get the flags for
         * @returns the item flags for the given index
         */
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        /**
         * Returns the data for the given role and section in the header with the specified orientation.
         * //Since the model has not headers, an empty QVariant is always returned. // REMOVE THIS LINE?
         *
         * @param section - The section
         * @param orientation - The orientation
         * @param role - The role
         * @returns the header data. Always empty.
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        // === Others ===

        /**
         * Returns the ModuleItem stored under the specified model index.
         *
         * @param index - The model index to get the ModuleItem from
         * @returns the module item at the specified index
         */
        ModuleItem* getItem(const QModelIndex& index) const;

        /**
         * Returns the index where the specified ModuleItem can be found.
         *
         * @param item - The ModuleItem to search for in the item model
         * @returns the model index of the specified ModuleItem
         */
        QModelIndex getIndex(const ModuleItem* const item) const;

        /**
         * Returns the ModuleItem for a specified id and type.
         *
         * @param module_id - The id of the ModuleItem
         * @param type - The type of the ModuleItem
         * @returns the ModuleItem with the specified id and type.
         */
        ModuleItem* getItem(const u32 id, ModuleItem::TreeItemType type = ModuleItem::TreeItemType::Module) const;

        /**
         * Initializes the item model using the global netlist object gNetlist.
         */
        void init();

        /**
         * Clears the item model and deletes all ModuleItems.
         */
        void clear();

        /**
         * Add a module to the item model. For the specified module a new ModuleItem is created and stored.
         *
         * @param id - The id of the module to add.
         * @param parent_module - The id of the parent module of the module to add.
         */
        void addModule(const u32 id, const u32 parent_module);

        /**
         * Add a gate to the item model. For the specified gate a new ModuleItem is created and stored.
         *
         * @param id - The id of the gate to add.
         * @param parent_module - The id of the parent module of the gate to add.
         */
        void addGate(const u32 id, const u32 parent_module);

        /**
         * Add a net to the item model. For the specified net a new ModuleItem is created and stored.
         *
         * @param id - The id of the net to add.
         * @param parent_module - The id of the parent module of the net to add.
         */
        void addNet(const u32 id, const u32 parent_module);

        /**
         * Recursively adds the given module with all of its submodules (and their submodules and so on...)
         * and the gates and nets of those modules to the item model.
         *
         * @param module - The module which should be added to the item model together with all its 
         *                  submodules, gates and nets.
         * @param added_nets - A set of ids of nets. It's used to keep track of the nets that have already been added 
         *                      to the item model during the recursion. You can pass an empty set to add all nets.
         */
        void addRecursively(const Module* module, QSet<u32>& added_nets);

        /**
         * Updates the parent of the ModuleItem corresponding to the specified module. 
         * The new parent must already be set in the Module object.
         * 
         * @param module - The module whose ModuleItem will be reattached to a new parent in the item model.
        */
        void changeParentModule(const Module* module);

        /**
         * Removes a module from the item model. The specified module MUST be contained in the item model.
         *
         * @param id - The id of the module to remove
         */
        void remove_module(const u32 id);

        /**
         * Removes a gate from the item model. The specified gate MUST be contained in the item model.
         *
         * @param id - The id of the gate to remove
         */
        void remove_gate(const u32 id);

        /**
         * Removes a net from the item model. The specified net MUST be contained in the item model.
         *
         * @param id - The id of the net to remove
         */
        void remove_net(const u32 id);

        /**
         * Updates the ModuleItem for the specified module. The specified module MUST be contained in the item model.
         *
         * @param id - The id of the module to update
         */
        void updateModule(const u32 id);

//        void addModule(u32 id, u32 parent_module);
//        void addRecursively(const std::vector<Module*>& modules);
//        void remove_module(u32 id);
//        void updateModule(u32 id);


        /**
         * Returns <b>true</b> if the item model is currently changed/updated. This is the case while adding and
         * removing modules to/from the item model. It can be used to ignore certain signals sent by the item model
         * while the model is modified.
         *
         * @returns <b>true</b> while the item model is modified. Returns <b>false</b> otherwise.
         */
        bool isModifying();

    private:
        ModuleItem* mTopModuleItem;

        QMap<u32, ModuleItem*> mModuleMap;
        QMap<u32, ModuleItem*> mGateMap;
        QMap<u32, ModuleItem*> mNetMap;
        std::array<QMap<u32, ModuleItem*>*, 3> mModuleItemMaps = {&mModuleMap, &mGateMap, &mNetMap};;

        bool mIsModifying;
    };
}    // namespace hal

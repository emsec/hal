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
        void clear() override;

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
         * and the gates those modules to the item model.
         *
         * @param module - The module which should be added to the item model together with all its
         *                  submodules, gates and nets.
         */
        void addRecursively(const Module* module);

        /**
         * Removes a module from the item model. The specified module MUST be contained in the item model.
         *
         * @param id - The id of the module to remove
         */
        void removeModule(const u32 id);

        /**
         * Removes a gate from the item model. The specified gate MUST be contained in the item model.
         *
         * @param id - The id of the gate to remove
         */
        void removeGate(const u32 id);

        /**
         * Removes a net from the item model.
         *
         * @param id - The id of the net to remove
         */
        void removeNet(const u32 id);

        /**
         * Handles the assigment of gates to modules. 
         * If the gate does not yet exist in the item model, a new one is created.
         * All nets, that are connected to the gate, will be updated.
        */
        void handleModuleGateAssinged(const u32 id, const u32 parent_module);

        /**
         * Updates the position of a net in the ModuleTree. 
         * The net will be placed under the deepest module, that contains all sources and destinations of the net.
         * If no suitable parent could be found, then the net will instead be placed under the top module.
         * If the net does not exist in the item model (e.g. it's a global net), then nothing is updated.
         * 
         * @param net The net whose source or destination might have changed.
        */
        void updateNet(const Net* net, const QHash<const Net*,ModuleItem*>* parentAssignment = nullptr);

        /**
         * Reattaches the ModuleItem corresponding to the specified module to a new parent item.
         * The new parent must already be set in the Module object.
         *
         * @param module - The module whose ModuleItem will be reattached to a new parent in the item model.
        */
        void updateModuleParent(const Module* module);

        /**
         * Updates the ModuleItem for the specified module. The specified module MUST be contained in the item model.
         *
         * @param id - The id of the module to update
         */
        void updateModuleName(const u32 id);

        /**
         * Updates the ModuleItem for the specified gate. The specified gate MUST be contained in the item model.
         *
         * @param id - The id of the gate to update
         */
        void updateGateName(const u32 id);

        /**
         * Updates the ModuleItem for the specified net. The specified gate MUST be contained in the item model.
         *
         * @param id - The id of the net to update
         */
        void updateNetName(const u32 id);

        /**
         * Returns <b>true</b> if the item model is currently changed/updated. This is the case while adding and
         * removing modules to/from the item model. It can be used to ignore certain signals sent by the item model
         * while the model is modified.
         *
         * @returns <b>true</b> while the item model is modified. Returns <b>false</b> otherwise.
         */
        bool isModifying();

        void debugDump() const;

    private Q_SLOTS:
        void handleModuleNameChanged(Module* mod);

        void handleModuleRemoved(Module* mod);

        void handleModuleCreated(Module* mod);

        /**
         * Moves the ModuleItem corresponding to the module under it's new parent ModuleItem.
         * The items for all nets, that have at least one source or one destination within the module,
         * will be updated afterwards.
         *
         * @param module The module whose parent has changed.
        */
        void handleModuleParentChanged(const Module* module);

        void handleModuleGateAssigned(Module* mod, u32 gateId);

        void handleModuleGateRemoved(Module* mod, u32 gateId);

        void handleModuleGatesAssignBegin(Module* mod, u32 numberGates);

        void handleModuleGatesAssignEnd(Module* mod, u32 numberGates);

        void handleGateNameChanged(Gate* gat);

        void handleNetCreated(Net* net);

        void handleNetRemoved(Net* net);

        void handleNetNameChanged(Net* net);

        void handleNetUpdated(Net* net, u32 data);
    private:
        /**
         * Searches for a new parent module, such that it is the deepest module in the hierarchy, that contains all
         * sources and destinations of the net. 
         * 
         * @param net The net for which a new parent should be searched.
         * 
         * @return The new parent module, that contains all sources and destinations of net. If no such parent could be found 
         * (e.g. net has no sources or destinations), nullptr is returned instead.
        */
        Module* findNetParent(const Net* net);
        void updateAllNets();
        void findNetParentRecursion(BaseTreeItem* parent, QHash<const Net*,ModuleItem*>& parentAssignment, std::unordered_set<Net*>& assignedNets) const;

        QMap<u32, ModuleItem*> mModuleMap;
        QMap<u32, ModuleItem*> mGateMap;
        QMap<u32, ModuleItem*> mNetMap;
        std::array<QMap<u32, ModuleItem*>*, 3> mModuleItemMaps = {&mModuleMap, &mGateMap, &mNetMap};;

        bool mIsModifying;
    };
}    // namespace hal

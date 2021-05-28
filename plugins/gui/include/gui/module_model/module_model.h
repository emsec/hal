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
#include "gui/gui_utils/sort.h"
#include "hal_core/netlist/module.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QColor>
#include <set>

namespace hal
{
    class ModuleItem;

    /**
     * @ingroup gui
     * @brief Represents the netlist module's hierarchy.
     *
     * The ModuleModel is the item model that represents the modules and their hierarchy in the netlist.
     */
    class ModuleModel : public QAbstractItemModel
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
         * Since the model has not headers, an empty QVariant is always returned.
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
         * @param index - The model index to get the module item from
         * @returns the module item at the specified index
         */
        ModuleItem* getItem(const QModelIndex& index) const;

        /**
         * Returns the module index where the specified ModuleItem can be found.
         *
         * @param item - The module item to search for in the item model
         * @returns the model index of the specified ModuleItem
         */
        QModelIndex getIndex(const ModuleItem* const item) const;

        /**
         * Returns the ModuleItem for a specified module id.
         *
         * @param module_id - The id of the module to get the ModuleItem for
         * @returns the ModuleItem with the id module_id
         */
        ModuleItem* getItem(const u32 module_id) const;

        /**
         * Initializes the item model using the global netlist object gNetlist.
         */
        void init();

        /**
         * Clears the item model.
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
         * Recursively adds all given modules with all their sub modules (and their submodules and so on...) to
         * the item model.
         *
         * @param modules - The list of modules which should be added to the item model together
         *                  with all their submodules.
         */
        void addRecursively(const std::vector<Module*>& modules);

        /**
         * Removes a module from the item model. The specified module MUST be contained in the item model.
         *
         * @param id - The id of the model to remove
         */
        void remove_module(const u32 id);

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

        /**
         * Gets the module color of a module of a specific id.
         *
         * @param id - The module id of the module to get the color for
         * @returns the color of the module
         */
        QColor moduleColor(u32 id) const;

        /**
         * Changes the color of a module.
         *
         * @param id - The id of the module
         * @param col - The new color
         * @returns the old color of the module (used to create an undo action easier)
         */
        QColor setModuleColor(u32 id, const QColor& col);

        /**
         * Changes the color of a module to a random color.
         *
         * @param id - The id of the module
         * @returns the old color of the module (used to create an undo action easier)
         */
        QColor setRandomColor(u32 id);

        /**
         * Removes the color that belongs to the given id.
         *
         * @param id - The module id for which to remove the color.
         */
        void removeColor(u32 id);

    private:
        ModuleItem* mTopModuleItem;

        QMap<u32, ModuleItem*> mModuleItems;
        QMap<u32, QColor> mModuleColors;

        bool mIsModifying;
    };
}    // namespace hal

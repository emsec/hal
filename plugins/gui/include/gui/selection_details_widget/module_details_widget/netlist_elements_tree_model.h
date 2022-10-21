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

#include <QAbstractItemModel>
#include <QIcon>
//#include "gui/new_selection_details_widget/models/tree_item.h"
#include "gui/basic_tree_model/base_tree_model.h"

namespace hal
{
    class Module;
    class Gate;
    class Net;
    class TreeItem;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief A model to display arbitrary elements of the netlist.
     */
    class NetlistElementsTreeModel : public BaseTreeModel
    {
        Q_OBJECT
    public:

        //metatype declaration at the end of file
        enum class itemType {module = 0, gate = 1, net = 2};

        /**
         * The constructor.
         *
         * @param parent - The model's parent.
         */
        NetlistElementsTreeModel(QObject* parent = nullptr);

        /**
          * The destructor.
          */
        ~NetlistElementsTreeModel();

        /** @name Overwritten model functions
         */
        ///@{

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        ///@}

        /**
         * Overwritten clear function to reset this model's specific structures.
         */
        void clear() override;

        /**
         * Sets the module's content to the specified ids. Adds all Elements to the top level of
         * the tree. As of now, the model does not check the consistency of the items. For instance, if
         * modules that shall explicity be displayed are already within other given modules. This also
         * applies to gates. This can result in duplicate items that are displayed at the top level as
         * well as within a module hirarchy.
         *
         * @param modIds - Ids of modules.
         * @param gateIds - Ids of gates.
         * @param netIds - Ids of nets.
         * @param displayModulesRecursive - True to add all submodules of the given module list to the tree.
         * @param showGatesInSubmods - True to show the gates in the submodules that were added if displayModulesRecursive = true.
         * @param showNetsInSubmods - True to show the net in the submodules that were added if displayModulesRecursive = true.
         */
        void setContent(QList<int> modIds, QList<int> gateIds, QList<int> netIds, bool displayModulesRecursive = true, bool showGatesInSubmods = true, bool showNetsInSubmods = true);

        /**
         * Updates the model's content to the given module. Convenient functions that can be used
         * instead of the more general setContent() function.
         *
         * @param mod - The module to display.
         * @param showGates - True to add gates, False to show only module hierarchy.
         * @param showNets - True to add nets, False to show only module hierarchy.
         * @param displayModulesRecursive - True to show
         */
        void setModule(Module* mod, bool showGates = true, bool showNets = true, bool displayModulesRecursive = true);

        /**
         * Get the type (enum) of a given item.
         *
         * @param item - The item for which the type is requested.
         * @return The item's type.
         */
        itemType getTypeOfItem(TreeItem* item) const;

        /**
         * Get the module/gate/net id that the given item represents.
         * To know the type of the item, call getTypeOfItem().
         *
         * @param item - The item from which to extract the id.
         * @return The corresponding module, gate, or net id.
         */
        int getRepresentedIdOfItem(TreeItem* item) const;

        /** @name Event Handler Functions
         */
        ///@{
        void gateNameChanged(Gate* g);
        void gateRemoved(Gate* g);
        void netNameChanged(Net* n);
        void netRemoved(Net* n);
        void moduleNameChanged(Module* m);
        void moduleTypeChanged(Module* m);
        void moduleSubmoduleRemoved(Module* m, int removed_module);
        //optional
        void moduleGateAssigned(Module* m, int assigned_gate); //const u32
        void moduleGateRemoved(Module* m, int removed_gate); //const u32 //same as assign_gate(top)
        void moduleSubmoduleAdded(Module* m, int added_module); 
        ///@}


        //Column identifier
        static const int sNameColumn = 0;
        static const int sIdColumn = 1;
        static const int sTypeColumn = 2;

        //additional data keys
        const QString keyItemType = "type"; //also save value in enum (if it is possible with QVariant)
        const QString keyRepresentedID = "id";


    Q_SIGNALS:

        /**
         * Signal that is emitted when the number of direct submodules changed
         * in the case the displayed content is displayed by using the method
         * setModule().
         *
         * @param newNumber - The new number of direct submodules.
         */
        void numberOfSubmodulesChanged(const int newNumber);

    private:
        //Note: make these somehow static (does not work with pointer...?)
        QIcon mModuleIcon;
        QIcon mGateIcon;
        QIcon mNetIcon;

        bool mGatesDisplayed;
        bool mNetsDisplayed;
        bool mDisplaySubmodRecursive;

        //boolean needed for a special case when displaying a module that is called with setModule
        bool mCurrentlyDisplayingModule;
        int mModId;

        //"2" options: //also: use QMultiMap in case multiple "same" items (gates etc) are displayed
        //1) 1 map that maps "raw element pointer (gate,net,module)" to a list of treeitems
        //2) 3 maps with either id->treeitems or pointer->treeitems
        //QMultiMap<void*, TreeItem*> mElementToTreeitem;
        QMultiMap<Module*, TreeItem*> mModuleToTreeitems;
        QMultiMap<Gate*, TreeItem*> mGateToTreeitems;
        QMultiMap<Net*, TreeItem*> mNetToTreeitems;

        //necessary because setModule uses beginResetModel (should not be called by each recursive iteration)
        void moduleRecursive(Module* mod, TreeItem* modItem,  bool showGates = true, bool showNets = true);

        /**
         * Utility function to determine the displayed icon for a given item
         *
         * @param item - The requested item.
         * @return A module, net, or gate icon depending on the item's type.
         */
        QIcon getIconFromItem(TreeItem* item) const;

        /**
         * Utility function to remove all net items of the given module item and
         * add the (potentionally) updated internal nets. Usually used when a gate is added
         * or removed from a gate (internal nets might change as a result).
         *
         * @param moduleItem - The module item to modify.
         */
        void updateInternalNetsOfModule(TreeItem* moduleItem);

    };

}

Q_DECLARE_METATYPE(hal::NetlistElementsTreeModel::itemType)

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

#include <QAbstractItemModel>
#include <QIcon>
#include "gui/new_selection_details_widget/models/tree_item.h"
#include "gui/new_selection_details_widget/models/base_tree_model.h"

namespace hal
{
    class Module;
    /**
     * @ingroup utility_widgets-selection_details
     * @brief A model to display arbitrary elements of the netlist.
     */
    class NetlistElementsTreeModel : public BaseTreeModel
    {
        Q_OBJECT
    public:
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


        //Column identifier
        static const int sNameColumn = 0;
        static const int sIdColumn = 1;
        static const int sTypeColumn = 2;

        enum  itemType{module = 0, gate = 1, net = 2};
        Q_ENUM(itemType)

    private:
        //TreeItem* mRootItem;
        //Note: make these somehow static (does not work with pointer...?)
        QIcon mModuleIcon;
        QIcon mGateIcon;
        QIcon mNetIcon;
        QString mItemTypeKey = "type"; //also save value in enum (if it is possible with QVariant)

        //necessary because setModule uses beginResetModel (should not be called by each recursive iteration)
        void moduleRecursive(Module* mod, TreeItem* modItem,  bool showGates = true, bool showNets = true);

        /**
         * Utility function to determine the displayed icon for a given item
         *
         * @param item - The requested item.
         * @return A module, net, or gate icon depending on the item's type.
         */
        QIcon getIconFromItem(TreeItem* item) const;

    };

}

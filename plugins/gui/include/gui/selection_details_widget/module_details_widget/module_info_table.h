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

#include "gui/selection_details_widget/general_table_widget.h"

namespace hal
{
    class Module;
    class Net;

    class ModuleInfoTable : public GeneralTableWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        ModuleInfoTable(QWidget* parent = nullptr);

        /**
        * Updates the table with the data of the module.
        *
        * @param module - The module.
        */
        void setModule(hal::Module* module);

    private:

        /**
         * Returns the module's name in a suitable manner for tables and the clipboard.
         *
         * @return name - The module's name.
         */
        QString name() const;

        /**
         * Returns the module's id in a suitable manner for tables and the clipboard.
         *
         * @return id - The module's id.
         */
        QString id() const;

        /**
         * Returns the module's type in a suitable manner for tables and the clipboard.
         *
         * @return type - The module's type.
         */        
        QString type() const;

        /**
         * Returns the module's parent module in a suitable manner for tables and the clipboard.
         *
         * @return parent module - The module's parent module.
         */
        QString parentModule() const;

        /**
         * Returns the total number of gates contained in the module.
         */
        QString numberOfAllGates() const;

        /**
         * Returns the number of gates directly contained in the module.
         */
        QString numberOfDirectGateMembers() const;

        /**
         * Returns the number of gates contained in all submodules.
         */
        QString numberOfGatesInSubmodules() const;

        /**
         * Returns the number of submodules contained in the module in a suitable mannfer for table and the clipboard.
         */
        QString numberOfSubModules() const;

        /**
         * Returns the number of nets contained in the module in a suitable mannfer for table and the clipboard.
         */
        QString numberOfNets() const;

        /**
         * Returns the module's number of pins.
         */
        QString numberOfPins() const;

        /**
         * Returns the module's number of pin groups.
         */
        QString numberOfPinGroups() const;

        /**
         * Returns the module's number of input nets.
         */
        QString numberOfInputNets() const;

        /**
         * Returns the module's number of output nets.
         */
        QString numberOfOutputNets() const;

        /**
         * Returns the module's number of internal nets.
         */
        QString numberOfInternalNets() const;

        /**
         * Returns True as a QString if the module is the top module, False otherwise.
         */
        QString isTopModule() const;

        /**
         * Opens a Dialog to change the modules's name.
         */
        void changeName();

        /**
         * Copies the module's name to the clipboard.
         */
        void copyName() const;

        /**
         * Copies the python code to retrieve the module's name to the clipboard.
         */
        void pyCopyName() const;

        /**
         * Copies the module's id to the clipboard.
         */
        void copyId() const;

        /**
         * Copies the python code to retrieve the module's id to the clipvoard.
         */
        void pyCopyId() const;

        /**
         * Opens a Dialog to change the modules's type.
         */
        void changeType();

        /**
         * Copies the module's type to the clipboard.
         */
        void copyType() const;

        /**
         * Copies the python code to retrieve the module's type to the clipboard.
         */
        void pyCopyType() const;

        /**
         * Copies the module's parent module to the clipboard.
         */
        void copyModule() const;

        /**
         * Copies the python code to retrieve the module's parent module to the clipboard.
         */
        void pyCopyModule() const;

        /**
         * Copies the number of gates contained in the module to the clipboard.
         */
        void copyNumberOfAllGates() const;

        /**
         * Copies the python code to retrieve all gates in the module and its submodules.
         */
        void pyCopyAllGates() const;

        /**
         * Copies the number of gates directly contained in the module.
         */
        void copyNumberOfDirectGates() const;

        /**
         * Copies the python code to retrieve all direct gates of the module
         */
        void pyCopyDirectMemberGates() const;

        /**
         * Copies the number of gates contained in all submodules.
         */
        void copyNumberOfGatesInSubmodules() const;

        /**
         * Copies the python code to retrieve only gates in the submodules.
         */
        void pyCopyGatesInSubmodules() const;

        /**
         * Copies the number of submodules contained in the module to the clipboard.
         */
        void copyNumberOfSubmodules() const;

        /**
         * Copies the python code to retrieve the submodules to the clipboard.
         */
        void pyCopyGetSubmodules() const;

        /**
         * Copies the number of ALL nets contained in the module to the clipboard.
         */
        void copyNumberOfNets() const;

        /**
         * Copies the python code to retrieve ALL nets to the clipboard.
         */
        void pyCopyGetNets() const;

        /**
         * Copies the number of pins to the clipboard.
         */
        void copyNumberOfPins() const;

        /**
         * Copies the python code to retrieve the module's pins to the clipboard.
         */
        void pyCopyGetPins() const;

        /**
         * Copies the number of pin groups to the clipboard.
         */
        void copyNumberOfPinGroups() const;

        /**
         * Copies the python code to retrieve the module's pin groups to the clipboard.
         */
        void pyCopyGetPinGroups() const;

        /**
         * Copies the module's number of inputs.
         */
        void copyNumberOfInputs() const;

        /**
         * Copies the python code to retrieve the module's input nets.
         */
        void pyCopyGetInputNets() const;

        /**
         * Copies the module's number of outputs.
         */
        void copyNumberOfOutputs() const;

        /**
         * Copies the python code to retrieve the module's output nets.
         */
        void pyCopyGetOutputNets() const;

        /**
         * Copies the module's number of internal nets to the clipboard.
         */
        void copyNumberOfInternalNets() const;

        /**
         * Copies the python code to retrieve the internal nets to the clipboard.
         */
        void pyCopyGetInternalNets() const;

        /**
         * Copies the python code to check if the module is the top module.
         */
        void pyCopyIsTopModule() const;

        /**
         * Copies the ID of the parent module to the clipboard.
         */
        void copyParentID() const;

        /**
         * Sets the parent module as the current selection.
         */
        void setParentAsSelection();

        /**
         * Adds the parent module to the current selection.
         */
        void addParentToSelection();

        /**
         * Changes the selection to the modules's parent module.
         */
        void navModule();

        /**
         * Uses the ModuleDialog to open a popup to change the module's parent.
         */
        void changeParentAction();

        /**
         * Handle relayed removal of a module.
         * 
         * @param module - Module which has been removed.
         */
        void handleModuleRemoved(Module* module);

        /**
         * Handle relayed change of a module. Handled changes include name change, type change and change of module type. 
         * 
         * @param module - Module which has been changed.
         */
        void handleModuleChanged(Module* module);

        /**
         * Handle relayed change of a submodule. Handled changes include addition and removal of submodules to another module.
         * 
         * @param module - Parent module of the changed module.
         * @param affectedModuleId - The id of the module which has been changed.
         */
        void handleSubmoduleChanged(Module* parentModule, u32 affectedModuleId);

        /**
         * Handle relayed change of a gate. Handled changes include addition and removal of gates to a module.
         * 
         * @param module - Parent module of the changed gate.
         * @param affectedGateId - The id of the gate which has been changed.
         */
        void handleGateChanged(Module* parentModule, u32 affectedGateId);

        /**
         * Handle relayed change of a net. Handled changes include source / destination addition and removal.
         * 
         * @param Net - Net which has been changed.
         * @param affectedGateId - The id of the gate which is / was source / destination.
         */
        void handleNetChaned(Net* net, u32 affectedGateId);

        /**
         * Refreshes the table with the data of the currently set gate.
         */
        void refresh();

        Module* mModule;

        QMenu* mNameEntryContextMenu;
        QMenu* mIdEntryContextMenu;
        QMenu* mTypeEntryContextMenu;
        QMenu* mModuleEntryContextMenu;
        QMenu* mNumOfAllGatesContextMenu;
        QMenu* mNumOfDirectGatesContextMenu;
        QMenu* mNumOfGatesInSubmodulesContextMenu;
        QMenu* mNumOfSubmodulesContextMenu;
        QMenu* mNumOfNetsContextMenu;
        QMenu* mNumOfPinsContextMenu;
        QMenu* mNumOfPinGroupsContextMenu;
        QMenu* mNumOfInputNetsContextMenu;
        QMenu* mNumOfOutputNetsContextMenu;
        QMenu* mNumOfInternalNetsContextMenu;
        QMenu* mIsTopModuleContextMenu;
        QAction* mChangeParentAction;//is disabled if current module is the top module

        std::function<void()> mModuleDoubleClickedAction;
        QIcon mPyIcon;

        static const QString nameRowKey;
        static const QString idRowKey;
        static const QString typeRowKey;
        static const QString moduleRowKey;
        static const QString noOfAllGatesRowKey;
        static const QString noOfDirectGatesRowKey;
        static const QString noOfGatesInSubmodulesRowKey;
        static const QString noOfModulesRowKey;
        static const QString noOfNetsRowKey;
        static const QString noOfPinsKey;
        static const QString noOfPinGroupsKey;
        static const QString noOfInputNetsKey;
        static const QString noOfOutputNetsKey;
        static const QString noOfInternalNetsKey;
        static const QString isTopModuleKey;

    }; 
}

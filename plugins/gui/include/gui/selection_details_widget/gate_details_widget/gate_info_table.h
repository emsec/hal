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

#include "gui/selection_details_widget/general_table_widget.h"

namespace hal
{
    class Gate;
    class Module;

    class GateInfoTable : public GeneralTableWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        GateInfoTable(QWidget* parent = nullptr);

        /**
        * Updates the table with the data of the gate.
        *
        * @param gate - The gate.
        */
        void setGate(Gate* gate);

    private:

        /**
         * Returns the gate's name in a suitable manner for tables and the clipboard.
         *
         * @return name - The gate's name.
         */
        QString name() const;
       
        /**
         * Returns the gate's id in a suitable manner for tables and the clipboard.
         *
         * @return id - The gate's id.
         */
        QString id() const;

        /**
         * Returns the gate's type in a suitable manner for tables and the clipboard.
         *
         * @return type - The gate's type.
         */
        QString type() const;

        /**
         * Returns the gate's type propierties in a suitable manner for tables and the clipboard.
         *
         * @return gate type properties - The gate's type properties.
         */
        QString properties() const;

        /**
         * Returns the gate's location in a suitable manner for tables and the clipboard.
         *
         * @return location - The gate's location.
         */
        QString location() const;

        /**
         * Returns the gate's parent module in a suitable manner for tables and the clipboard.
         *
         * @return parent module - The gate's parent module.
         */
        QString parentModule() const; 

        /**
         * Opens a Dialog to change the gate's name.
         */
        void changeName();

        /**
         * Copies the gate's name to the clipboard.
         */
        void copyName() const;

        /**
         * Copies the python code to retrieve the gate's name to the clipboard.
         */
        void pyCopyName() const;

        /**
         * Copies the gate's id to the clipboard.
         */
        void copyId() const;

        /**
         * Copies the python code to retrieve the gate's id.
         */
        void pyCopyId() const;

        /**
         * Copies the gate's type to the clipboard.
         */
        void copyType() const;

        /**
         * Copies the python code to retrieve the gate's type to the clipboard.
         */
        void pyCopyType() const;

        /**
         * Copies the gate's type properties to the clipboard.
         */
        void copyproperties() const;

        /**
         * Copies the python code to retrieve the gate's type properties to the clipboard.
         */
        void pyCopyproperties() const;

        /**
         * Copies the gate's location to the clipboard.
         */
        void copyLocation() const;

        /**
         * Copies the python code to retrieve the gate's location to the clipboard.
         */
        void pyCopyLocation() const;
        
        /**
         * Copies the gate's parent module's name to the clipboard.
         */
        void copyModuleName() const;

        /**
         * Copies the python code to retrieve the gate's parent module to the clipboard.
         */
        void pyCopyModule() const;

        /**
         * Copies the id of the gate's module to the clipboard.
         */
        void copyModuleID() const;

        /**
         * Adds the gate's module to the current selection.
         */
        void addModuleToSelection();

        /**
         * Changes the selection to the gate's parent module.
         */
        void setModuleAsSelection();

        /**
         * Uses the ModuleDialog to open a popup for the Change module action.
         */
        void moveToModuleAction();

        /**
         * Handle relayed removal of a gate.
         * 
         *  @param gate - Gate which has been removed.
         */
        void handleGateRemoved(Gate* gate);
        
        /**
         * Handle relayed name change of a gate.
         * 
         * @param gate - Gate which name has been changed.
         */
        void handleGateNameChanged(Gate* gate);

        /**
         * Handle relayed location change of a gate.
         * 
         * @param gate - Gate which location has been changed.
         */
        void handleGateLocationChanged(Gate* gate);

        /**
         * Handle relayed name change of a module.
         * 
         * @param - Module which name has been changed.
         */
        void handleModuleNameChanged(Module* module);

        /**
         * Handle relayed change of a gate assignment.
         * 
         * @param module - Module which a gate has been asssigned to.
         * @param gateId - The id of the assigned gate.
         */
        void handleModuleGateAssigned(Module* module, const u32 gateId);

        /**
         * Refreshes the table with the data of the currently set gate.
         */
        void refresh();

        Gate* mGate;

        QMenu* mNameEntryContextMenu;
        QMenu* mIdEntryContextMenu;
        QMenu* mTypeEntryContextMenu;
        QMenu* mPropertiesEntryContextMenu;
        QMenu* mLocationEntryContextMenu;
        QMenu* mModuleEntryContextMenu;

        std::function<void()> mModuleDoubleClickedAction;

        static const QString nameRowKey;
        static const QString idRowKey;
        static const QString typeRowKey;
        static const QString gateTypePropertiesRowKey;
        static const QString locationRowKey;
        static const QString moduleRowKey;
    };
}

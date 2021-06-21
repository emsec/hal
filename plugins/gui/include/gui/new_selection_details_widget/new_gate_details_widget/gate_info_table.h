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

#include "gui/new_selection_details_widget/general_table_widget.h"

namespace hal
{
    class Gate;

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
        * Updates the table with the data of the gate with the given id.
        *
        * @param gateId - The id of the gate.
        */
        void update(u32 gateId);

    private:

        Gate* mGate;

        QString name() const;
        QString id() const;
        QString type() const;
        QString properties() const;
        QString location() const;
        QString module() const; 

        void changeName();
        void copyName() const;
        void pyCopyName() const;

        void copyId() const;

        void copyType() const;
        void pyCopyType() const;

        void copyproperties() const;
        void pyCopyproperties() const;

        void copyLocation() const;
        void pyCopyLocation() const;
        
        void copyModule() const;
        void pyCopyModule() const;
        void navModule();

        QMenu* mNameEntryContextMenu;
        QMenu* mIdEntryContextMenu;
        QMenu* mTypeEntryContextMenu;
        QMenu* mPropertiesEntryContextMenu;
        QMenu* mLocationEntryContextMenu;
        QMenu* mModuleEntryContextMenu;

        std::function<void()> mModuleDoubleClickedAction;
    };
}

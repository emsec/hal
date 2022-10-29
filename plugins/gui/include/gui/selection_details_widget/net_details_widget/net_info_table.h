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

#include "gui/selection_details_widget/general_table_widget.h"

namespace hal
{
    class Netlist;
    class Net;

    class NetInfoTable : public GeneralTableWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        NetInfoTable(QWidget* parent = nullptr);

        /**
        * Updates the table with the data of the net.
        *
        * @param net - The net.
        */
        void setNet(Net* net);

    private:

        /**
         * Returns the net's name in a suitable manner for tables and the clipboard.
         *
         * @return name - The net's name.
         */
        QString name() const;

        /**
         * Returns the net's id in a suitable manner for tables and the clipboard.
         *
         * @return id - The net's id.
         */
        QString id() const;

        /**
         * Returns the net's type in a suitable manner for tables and the clipboard.
         *
         * @return type - The net's type.
         */
        QString type() const;

        /**
         * Returns the net's number of sources in a suitable manner for tables and the clipboard.
         *
         * @return name - The net's name.
         */
        QString numberOfSrcs() const;

        /**
         * Returns the net's name in a suitable manner for tables and the clipboard.
         *
         * @return name - The net's name.
         */
        QString numberOfDsts() const;

        /**
         * Opens a Dialog to change the net's name.
         */
        void changeName();

        /**
         * Copies the net's name to the clipboard.
         */
        void copyName() const;

        /**
         * Copies the python code to retrieve the net's name to the clipboard.
         */
        void pyCopyName() const;

        /**
         * Copies the net's id to the clipboard.
         */
        void copyId() const;

        /**
         * Copies the python code to retrieve the net's id to the clipboard.
         */
        void pyCopyId() const;

        /**
         * Copies the net's type to the clipboard.
         */
        void copyType() const;

        /**
         * Copies the python code to retrieve the net's type to the clipboard.
         */
        void pyCopyType() const;
        
        /**
         * Copies the net's number of sources to the clipboard.
         */
        void copyNumberOfSrcs() const;

        /**
         * Copies the python code to retrieve the net's sources to the clipboard.
         */
        void pyCopySrcs() const;

        /**
         * Copies the net's number of destinations to the clipboard.
         */
        void copyNumberOfDsts() const;

        /**
         * Copies the python code to retrieve the net's destinations to the clipboard.
         */
        void pyCopyDsts() const;

        /**
         * Handle relayed removal of a net.
         * 
         *  @param net - Net which has been removed.
         */
        void handleNetRemoved(Net* net);

        /**
         * Handle relayed name change of a net.
         * 
         * @param net - Net which name has been changed.
         */
        void handleNetNameChanged(Net* net);

        /**
         * Handle relayed type change of a net.
         * 
         * @param netlist - Netlist which contains the net.
         * @param netId - Id of the net which type has been changed.
         */
        void handleNetTypeChanged(Netlist* netlist, const u32 netId);

        /**
         * Handle relayed source / destination change of a net.
         * 
         * @param net - Net which source / destination has been changed.
         * @param netId - Id of the source / destination gate.
         */
        void handleSrcDstChanged(Net* net, u32 srcDstGateId);

        /**
         * Refreshes the table with the data of the currently set net.
         */
        void refresh();

        Net* mNet;

        QMenu* mNameEntryContextMenu;
        QMenu* mIdEntryContextMenu;
        QMenu* mTypeEntryContextMenu;
        QMenu* mNumSrcsEntryContextMenu;
        QMenu* mNumDstsEntryContextMenu;

        static const QString nameRowKey;
        static const QString idRowKey;
        static const QString typeRowKey;
        static const QString noOfSrcRowKey;
        static const QString noOfDstRowKey;
    };
}

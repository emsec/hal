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

#include "gui/netlist_relay/netlist_relay.h"

#include <QObject>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Observes changes of the netlist and notifies the FileStatusManager.
     *
     * The NetlistWatcher observes changes of the netlist. Its main purpose is to notify the FileStatusManager about
     * any changes that occur (especially outside the gui). Afterwards the FileStatusManager marks the netlist as
     * modified until the next save.
     * TODO: Why does the FileStatusManager not use the NetlistRelay instead?
     */
    class NetlistWatcher : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         * Its registers all its callbacks to the core.
         *
         * @param parent - The parent object
         */
        NetlistWatcher(QObject* parent = nullptr);

        /**
         * Destructor.
         * Unregisters all registered callbacks.
         */
        ~NetlistWatcher();

        /**
         * The FileStatusManager is only notified once. Afterwards this function has to be called to enable the
         * notification again.
         * TODO: Only one notification ever? (Only called by constructor...)
         */
        void reset();

    private:
        void handleNetlistEvent( netlist_event_handler::event ev, Netlist* object, u32 associated_data);
        void handleModuleEvent(  ModuleEvent::event   ev, Module*   mod, u32 associated_data);
        void handleGateEvent(    GateEvent::event     ev, Gate*     gat, u32 associated_data);
        void handleNetEvent(     NetEvent::event      ev, Net*      net, u32 associated_data);
        void handleGroupingEvent(GroupingEvent::event ev, Grouping* grp, u32 associated_data);

        void handleNetlistModified();
        bool mNotified;
    };
}

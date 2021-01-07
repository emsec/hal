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

#include "hal_core/utilities/callback_hook.h"

/**
 * @ingroup events
 */

// TODO convert namespace to class

namespace hal
{
    class Netlist;

    class EventHandler
    {
        
        public:
            /**
             * Enables/disables callbacks for this handler.<br>
             * Enabled by default.
             *
             * @param[in] flag - True to enable, false to disable.
             */
            NETLIST_API void enable(bool flag);

            // TODO add other events

            enum gate_event
            {
                created,            ///< no associated_data
                removed,            ///< no associated_data
                name_changed,       ///< no associated_data
                location_changed    ///< no associated_data
            };

            enum grouping_event
            {
                created,            ///< no associated_data
                removed,            ///< no associated_data
                name_changed,       ///< no associated_data
                gate_assigned,      ///< associated_data = id of inserted gate
                gate_removed,       ///< associated_data = id of removed gate
                net_assigned,       ///< associated_data = id of inserted net
                net_removed,        ///< associated_data = id of removed net
                module_assigned,    ///< associated_data = id of inserted module
                module_removed,     ///< associated_data = id of removed module
            };

            enum module_event
            {
                created,                     ///< no associated_data
                removed,                     ///< no associated_data
                name_changed,                ///< no associated_data
                type_changed,                ///< no associated_data
                parent_changed,              ///< no associated_data
                submodule_added,             ///< associated_data = id of added module
                submodule_removed,           ///< associated_data = id of removed module
                gate_assigned,               ///< associated_data = id of inserted gate
                gate_removed,                ///< associated_data = id of removed gate
                input_port_name_changed,     ///< associated_data = id of respective net
                output_port_name_changed,    ///< associated_data = id of respective net
            };

            enum net_event
            {
                created,         ///< no associated_data
                removed,         ///< no associated_data
                name_changed,    ///< no associated_data
                src_added,       ///< associated_data = id of src gate
                src_removed,     ///< associated_data = id of src gate
                dst_added,       ///< associated_data = id of dst gate
                dst_removed      ///< associated_data = id of dst gate
            };

            enum netlist_event
            {
                id_changed,                ///< associated_data = old id
                input_filename_changed,    ///< no associated_data
                design_name_changed,       ///< no associated_data
                device_name_changed,       ///< no associated_data
                marked_global_vcc,         ///< associated_data = id of gate
                marked_global_gnd,         ///< associated_data = id of gate
                unmarked_global_vcc,       ///< associated_data = id of gate
                unmarked_global_gnd,       ///< associated_data = id of gate
                marked_global_input,       ///< associated_data = id of net
                marked_global_output,      ///< associated_data = id of net
                marked_global_inout,       ///< associated_data = id of net
                unmarked_global_input,     ///< associated_data = id of net
                unmarked_global_output,    ///< associated_data = id of net
                unmarked_global_inout,     ///< associated_data = id of net

            };

            //TODO add other notify/register functions


            /**
             * Executes all registered callbacks.
             *
             * @param[in] ev - the event which occured.
             * @param[in] netlist - The affected object.
             * @param[in] associated_data - may have a meaning depending on the event type.
             */
            NETLIST_API void notify(netlist_event ev, Netlist* netlist, u32 associated_data = 0xFFFFFFFF);
            NETLIST_API void notify(net_event ev, Net* net, u32 associated_data = 0xFFFFFFFF);
            NETLIST_API void notify(module_event ev, Module* module, u32 associated_data = 0xFFFFFFFF);
            NETLIST_API void notify(grouping_event ev, Grouping* grouping, u32 associated_data = 0xFFFFFFFF);
            NETLIST_API void notify(gate_event ev, Gate* gate, u32 associated_data = 0xFFFFFFFF);



            /**
             * Registers a callback function.
             *
             * @param[in] name - name of the callback, used for callback removal.
             * @param[in] function - The callback function.
             */
            NETLIST_API void register_callback(const std::string& name, std::function<void(netlist_event e, Netlist* netlist, u32 associated_data)> function);
            NETLIST_API void register_callback(const std::string& name, std::function<void(net_event e, Net*, u32 associated_data)> function);
            NETLIST_API void register_callback(const std::string& name, std::function<void(module_event e, Module* module, u32 associated_data)> function);
            NETLIST_API void register_callback(const std::string& name, std::function<void(grouping_event e, Grouping* grouping, u32 associated_data)> function);
            NETLIST_API void register_callback(const std::string& name, std::function<void(gate_event e, Gate*, u32 associated_data)> function);


            /**
             * Removes a callback function.
             *
             * @param[in] name - name of the callback.
             */
            NETLIST_API void unregister_callback(const std::string& name);

    };    // class event_handler
}    // namespace hal

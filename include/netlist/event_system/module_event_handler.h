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

#include "core/callback_hook.h"

class netlist;

class module;

/**
 * @ingroup handler
 */
namespace module_event_handler
{
    /**
     * Enables/disables callbacks for this handler.<br>
     * Enabled by default.
     *
     * @param[in] flag - True to enable, false to disable.
     */
    NETLIST_API void enable(bool flag);

    enum event
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

    /**
    * Executes all registered callbacks.
    *
    * @param[in] ev - the event which occured.
    * @param[in] module - The affected object.
    * @param[in] associated_data - may have a meaning depending on the event type.
    */
    NETLIST_API void notify(event ev, std::shared_ptr<module> module, u32 associated_data = 0xFFFFFFFF);

    /**
     * Registers a callback function.
     *
     * @param[in] name - name of the callback, used for callback removal.
     * @param[in] function - The callback function.
     */
    NETLIST_API void register_callback(const std::string& name, std::function<void(event e, std::shared_ptr<module> module, u32 associated_data)> function);

    /**
     * Removes a callback function.
     *
     * @param[in] name - name of the callback.
     */
    NETLIST_API void unregister_callback(const std::string& name);

}    // namespace module_event_handler

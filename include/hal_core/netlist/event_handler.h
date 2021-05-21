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
#include "hal_core/utilities/enums.h"

#include <iostream>

// TODO convert namespace to class

namespace hal
{
    /** forward declaration */
    class Netlist;
    class Net;
    class Gate;
    class Module;
    class Grouping;

    class NetlistEvent
    {
    public:
        enum class event
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
    };

    template<>
    std::vector<std::string> EnumStrings<NetlistEvent::event>::data;

    class GateEvent
    {
    public:
        enum class event
        {
            created,            ///< no associated_data
            removed,            ///< no associated_data
            name_changed,       ///< no associated_data
            location_changed    ///< no associated_data
        };
    };

    template<>
    std::vector<std::string> EnumStrings<GateEvent::event>::data;

    class NetEvent
    {
    public:
        enum class event
        {
            created,         ///< no associated_data
            removed,         ///< no associated_data
            name_changed,    ///< no associated_data
            src_added,       ///< associated_data = id of src gate
            src_removed,     ///< associated_data = id of src gate
            dst_added,       ///< associated_data = id of dst gate
            dst_removed      ///< associated_data = id of dst gate
        };
    };

    template<>
    std::vector<std::string> EnumStrings<NetEvent::event>::data;

    class ModuleEvent
    {
    public:
        enum class event
        {
            created,                    ///< no associated_data
            removed,                    ///< no associated_data
            name_changed,               ///< no associated_data
            type_changed,               ///< no associated_data
            parent_changed,             ///< no associated_data
            submodule_added,            ///< associated_data = id of added module
            submodule_removed,          ///< associated_data = id of removed module
            gate_assigned,              ///< associated_data = id of inserted gate
            gate_removed,               ///< associated_data = id of removed gate
            input_port_name_changed,    ///< associated_data = id of respective net
            output_port_name_changed    ///< associated_data = id of respective net
        };
    };

    template<>
    std::vector<std::string> EnumStrings<ModuleEvent::event>::data;

    class GroupingEvent
    {
    public:
        enum class event
        {
            created,            ///< no associated_data
            removed,            ///< no associated_data
            name_changed,       ///< no associated_data
            gate_assigned,      ///< associated_data = id of inserted gate
            gate_removed,       ///< associated_data = id of removed gate
            net_assigned,       ///< associated_data = id of inserted net
            net_removed,        ///< associated_data = id of removed net
            module_assigned,    ///< associated_data = id of inserted module
            module_removed      ///< associated_data = id of removed module
        };
    };

    template<>
    std::vector<std::string> EnumStrings<GroupingEvent::event>::data;

    class EventHandler
    {
    private:
        CallbackHook<void(NetlistEvent::event, Netlist*, u32)> m_netlist_callback;
        CallbackHook<void(ModuleEvent::event, Module*, u32)> m_module_callback;
        CallbackHook<void(GateEvent::event, Gate*, u32)> m_gate_callback;
        CallbackHook<void(NetEvent::event, Net*, u32)> m_net_callback;
        CallbackHook<void(GroupingEvent::event, Grouping*, u32)> m_grouping_callback;
        bool netlist_event_enabled;
        bool module_event_enabled;
        bool gate_event_enabled;
        bool net_event_enabled;
        bool grouping_event_enabled;

    public:
        EventHandler();

        /**
             * Enables/disables callbacks for all handler.<br>
             * Enabled by default.
             *
             * @param[in] flag - True to enable, false to disable.
             */
        NETLIST_API void event_enable_all(bool flag);
        //TODO add other notify/register functions

        /**
             * Executes all registered callbacks.
             *
             * @param[in] ev - the event which occured.
             * @param[in] netlist - The affected object.
             * @param[in] associated_data - may have a meaning depending on the event type.
             */
        NETLIST_API void notify(NetlistEvent::event ev, Netlist* netlist, u32 associated_data = 0xFFFFFFFF);

        /**
             * Executes all registered callbacks.
             *
             * @param[in] ev - the event which occured.
             * @param[in] module - The affected object.
             * @param[in] associated_data - may have a meaning depending on the event type.
             */
        NETLIST_API void notify(ModuleEvent::event ev, Module* module, u32 associated_data = 0xFFFFFFFF);

        /**
             * Executes all registered callbacks.
             *
             * @param[in] ev - the event which occured.
             * @param[in] gate - The affected object.
             * @param[in] associated_data - may have a meaning depending on the event type.
             */
        NETLIST_API void notify(GateEvent::event ev, Gate* gate, u32 associated_data = 0xFFFFFFFF);

        /**
             * Executes all registered callbacks.
             *
             * @param[in] ev - the event which occured.
             * @param[in] net - The affected object.
             * @param[in] associated_data - may have a meaning depending on the event type.
             */
        NETLIST_API void notify(NetEvent::event ev, Net* net, u32 associated_data = 0xFFFFFFFF);

        /**
             * Executes all registered callbacks.
             *
             * @param[in] ev - the event which occured.
             * @param[in] grouping - The affected object.
             * @param[in] associated_data - may have a meaning depending on the event type.
             */
        NETLIST_API void notify(GroupingEvent::event ev, Grouping* grouping, u32 associated_data = 0xFFFFFFFF);

        /**
             * Registers a callback function.
             *
             * @param[in] name - name of the callback, used for callback removal.
             * @param[in] function - The callback function.
             */
        NETLIST_API void register_callback(const std::string& name, std::function<void(NetlistEvent::event e, Netlist* netlist, u32 associated_data)> function);

        /**
             * Registers a callback function.
             *
             * @param[in] name - name of the callback, used for callback removal.
             * @param[in] function - The callback function.
             */
        NETLIST_API void register_callback(const std::string& name, std::function<void(ModuleEvent::event e, Module* module, u32 associated_data)> function);

        /**
             * Registers a callback function.
             *
             * @param[in] name - name of the callback, used for callback removal.
             * @param[in] function - The callback function.
             */
        NETLIST_API void register_callback(const std::string& name, std::function<void(GateEvent::event e, Gate*, u32 associated_data)> function);

        /**
             * Registers a callback function.
             *
             * @param[in] name - name of the callback, used for callback removal.
             * @param[in] function - The callback function.
             */
        NETLIST_API void register_callback(const std::string& name, std::function<void(NetEvent::event e, Net*, u32 associated_data)> function);

        /**
             * Registers a callback function.
             *
             * @param[in] name - name of the callback, used for callback removal.
             * @param[in] function - The callback function.
             */
        NETLIST_API void register_callback(const std::string& name, std::function<void(GroupingEvent::event e, Grouping* grouping, u32 associated_data)> function);

        /**
             * Removes a callback function.
             *
             * @param[in] name - name of the callback.
             */
        NETLIST_API void unregister_callback(const std::string& name);

    };    // class event_handler
}    // namespace hal

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

#include "hal_core/defines.h"
#include "hal_core/utilities/callback_hook.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"

namespace hal
{
    class Netlist;

    /**
     * Plugin manager to load and unload plugins
     *
     * @ingroup utilities
     */
    namespace hal_file_manager
    {
        /**
         * Starts serialization to a .hal file.<br>
         * This fires all registered on_serialize callbacks.
         *
         * @param[in] file - The file path.
         * @param[in] netlist - The netlist to be serialized
         * @param[in] document - The JSON document to serialize to.
         * @returns True if all registered callbacks succeed.
         */
        bool serialize(const std::filesystem::path& file, Netlist* netlist, rapidjson::Document& document);

        /**
         * Starts deserialization of a .hal file.<br>
         * This fires all registered on_deserialize callbacks.
         *
         * @param[in] file - The file path.
         * @param[inout] netlist - The netlist to be obtained
         * @param[in] document - The JSON document to deserialize from.
         * @returns True if all registered callbacks succeed.
         */
        bool deserialize(const std::filesystem::path& file, Netlist* netlist, rapidjson::Document& document);

        /**
         * Add a callback to notify when a .hal file is being serialized.
         *
         * @param[in] identifier - An identifier name that can be used to remove the callback later.
         * @param[in] callback - The callback function. Parameters are:
         * * bool - Returns whether data was successfully extended, saving is aborted if False.
         * * const std::filesystem::path& - The hal file.
         * * rapidjson::Document& - The content to fill.
         */
        void register_on_serialize_callback(const std::string& identifier, std::function<bool(const std::filesystem::path&, Netlist*, rapidjson::Document&)> callback);

        /**
         * Removes a callback to notify when a .hal file is being serialized.
         *
         * @param[in] identifier - The identifier name given while registering
         */
        void unregister_on_serialize_callback(const std::string& identifier);

        /**
         * Add a callback to notify when a .hal file is being deserialized.
         *
         * @param[in] identifier - An identifier name that can be used to remove the callback later.
         * @param[in] callback - The callback function. Parameters are:
         * * bool - Returns whether data was successfully read, loading is aborted if False.
         * * const std::filesystem::path& - The hal file.
         * * rapidjson::Document& - The content to fill.
         */
        void register_on_deserialize_callback(const std::string& identifier, std::function<bool(const std::filesystem::path&, Netlist*, rapidjson::Document&)> callback);

        /**
         * Removes a callback to notify when a .hal file is being deserialized.
         *
         * @param[in] identifier - The identifier name given while registering
         */
        void unregister_on_deserialize_callback(const std::string& identifier);

    }    // namespace hal_file_manager
}    // namespace hal

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

#include "hal_core/defines.h"

#include <functional>
#include <map>
#include <set>
#include <string>

#define CALLBACK_HOOK_INVALID_IDX 0x0

namespace hal
{
    template<class>
    class CallbackHook;

    /**
     * @ingroup core
     */
    template<class R, class... ArgTypes>
    class CallbackHook<R(ArgTypes...)>
    {
    public:
        /**
         * Add a new callback function using an id.<br>
         * If the desired id for the callback is already registered, the previously registered callback is overwritten.
         *
         * @param[in] callback - The function to add.
         * @param[in] id - The desired callback id.
         * @returns The id of the callback.
         */
        u64 add_callback(const std::function<R(ArgTypes...)>& callback, u64 id = CALLBACK_HOOK_INVALID_IDX)
        {
            if (id == CALLBACK_HOOK_INVALID_IDX)
            {
                id = 1;
                while (m_callbacks.find(id) != m_callbacks.end())
                {
                    ++id;
                }
            }
            m_callbacks[id] = callback;
            return id;
        }

        /**
         * Add a new callback function using a string identifier.<br>
         * If the desired name for the callback is already taken, the previously registered callback is overwritten.
         *
         * @param[in] name - The desired callback identifier.
         * @param[in] callback - The function to add.
         */
        void add_callback(const std::string& name, const std::function<R(ArgTypes...)>& callback)
        {
            auto it = m_name_to_id_map.find(name);
            if (it != m_name_to_id_map.end())
            {
                remove_callback(it->second);
            }
            m_name_to_id_map[name] = add_callback(callback);
        }

        /**
         * Remove a callback function by id.
         *
         * @param[in] id - The id of the callback to delete.
         */
        void remove_callback(const u64 id)
        {
            auto it = m_callbacks.find(id);
            if (it != m_callbacks.end())
            {
                m_callbacks.erase(it);
                for (auto it2 = m_name_to_id_map.begin(); it2 != m_name_to_id_map.end(); it2++)
                {
                    if ((*it2).second == id)
                    {
                        m_name_to_id_map.erase(it2);
                        break;
                    }
                }
            }
        }

        /**
         * Remove a callback function by string identifier.
         *
         * @param[in] id - The identifier of the callback to delete.
         */
        void remove_callback(const std::string& id)
        {
            auto it = m_name_to_id_map.find(id);
            if (it != m_name_to_id_map.end())
            {
                remove_callback((*it).second);
            }
        }

        /**
         * Execute all registered callback functions with the provided arguments.
         *
         * @param[in] args - The arguments for the callback functions.
         */
        void inline operator()(ArgTypes... args)
        {
            if (this->size() == 0)
            {
                return;
            }
            for (const auto& i : m_callbacks)
            {
                (i.second)(args...);
            }
        }

        /**
         * Execute a specific callback functions with the provided arguments.<br>
         * If no callback is registered, a default-constructed object is returned.
         *
         * @param[in] idx - The id of the callback function.
         * @param[in] args - The arguments for the callback functions.
         * @returns The return value of the callback function.
         */
        R inline call(const u64 idx, ArgTypes... args)
        {
            auto it = m_callbacks.find(idx);
            if (it == m_callbacks.end())
            {
                return R();
            }
            return (it->second)(args...);
        }

        /**
         * Execute a specific callback functions with the provided arguments.<br>
         * If no callback is registered, a default-constructed object is returned.
         *
         * @param[in] idx - The string identifier of the callback function.
         * @param[in] args - The arguments for the callback functions.
         * @returns The return value of the callback function.
         */
        R inline call(const std::string& idx, ArgTypes... args)
        {
            auto it = m_name_to_id_map.find(idx);
            if (it == m_name_to_id_map.end())
            {
                return R();
            }
            return call((*it).second, args...);
        }

        /**
         * Check whether a callback function is registered for a given id.
         *
         * @param[in] id - The id of the callback function.
         * @returns True, if the callback function is registered.
         */
        bool is_callback_registered(const u64 id)
        {
            return m_callbacks.find(id) != m_callbacks.end();
        }

        /**
         * Check whether a callback function is registered for a given string identifier.
         *
         * @param[in] name - The identifier of the callback function.
         * @returns True, if the callback function is registered.
         */
        bool is_callback_registered(const std::string& name)
        {
            return m_name_to_id_map.find(name) != m_name_to_id_map.end();
        }

        /**
         * Get the number of registered callbacks.
         *
         * @returns The amount of all registered callback functions.
         */
        size_t size()
        {
            return m_callbacks.size();
        }

        /**
         * Get the ids of all registered callbacks.
         *
         * @returns The ids of all registered callback functions.
         */
        std::set<u64> get_ids()
        {
            std::set<u64> res;
            for (const auto& it : m_callbacks)
            {
                res.insert(it.first);
            }
            return res;
        }

        /**
         * Get the name of a registered callback.
         *
         * @param[in] id - The id to look up.
         * @returns The name or an empty string if no callback was registered with the given name.
         */
        std::string get_name(u64 id)
        {
            for (const auto& it : m_name_to_id_map)
            {
                if (it.second == id)
                {
                    return it.first;
                }
            }
            return "";
        }

    private:
        std::map<u64, std::function<R(ArgTypes...)>> m_callbacks;

        std::map<std::string, u64> m_name_to_id_map;
    };
}    // namespace hal

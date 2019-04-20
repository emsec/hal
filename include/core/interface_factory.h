//  MIT License
//
//  Copyright (c) 2019 Marc Fyrbiak
//  Copyright (c) 2019 Sebastian Wallat
//  Copyright (c) 2019 Max Hoffmann
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

#include "pragma_once.h"
#ifndef __HAL_INTERFACE_FACTORY_H__
#define __HAL_INTERFACE_FACTORY_H__

#include "core/interface_base.h"
#include "def.h"

#include <memory>

/**
 * @ingroup core
 */
class CORE_API i_factory
{
public:
    i_factory()          = default;
    virtual ~i_factory() = default;

    /**
     * Gets all available plugin types.
     *
     * @returns A set of all plugin types.
     */
    std::set<interface_type> get_plugin_types();

    /**
     * Get all plugin dependencies of this plugin.
     *
     * @return A set of plugins that this plugin depends on.
     */
    virtual std::set<std::string> get_dependencies();

    /**
     * Returns an interface for a specific type.
     *
     * @param[in] type Type of interface.
     * @return A shared pointer to the interface.
     */
    std::shared_ptr<i_base> query_interface(const interface_type type = interface_type::base);

protected:
    // Returns instance of a plugin
    virtual std::shared_ptr<i_base> get_plugin_instance() = 0;
};

/** typedef for i_factory constructor */
typedef i_factory* (*get_factory_fn_ptr_t)();

#endif /* __HAL_INTERFACE_FACTORY_H__ */

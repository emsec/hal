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

#ifndef __HAL_GATE_DECORATOR_H__
#define __HAL_GATE_DECORATOR_H__

#include "gate_decorator_system.h"
#include <string>

/* forward declaration */
class gate;

/**
 * @ingroup gate_decorator_system
 */
class NETLIST_API gate_decorator
{
public:
    /**
     * Creates a new decorator for a specific gate.
     *
     * @param[in] gate - The target gate.
     */
    explicit gate_decorator(std::shared_ptr<gate> const gate);

    virtual ~gate_decorator() = default;

    /**
     * Get the type of the decorator. <br>
     * Has to be implemented by subclasses.
     *
     * @returns The type.
     */
    virtual gate_decorator_system::decorator_type get_type() = 0;

    /**
     * Get the gate this decorator is created for.
     *
     * @returns The gate.
     */
    std::shared_ptr<gate> get_gate();

private:
    std::shared_ptr<gate> m_gate;
};

#endif /* __HAL_GATE_DECORATOR_H__ */

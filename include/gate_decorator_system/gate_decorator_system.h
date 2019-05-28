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

#ifndef __HAL_GATE_DECORATOR_SYSTEM_H__
#define __HAL_GATE_DECORATOR_SYSTEM_H__

#include "core/interface_base.h"

#include <functional>
#include <map>
#include <memory>

/* forward declaration */
class netlist;
class gate;
class gate_decorator;
class bdd;

/**
 * @ingroup gate_decorator_system
 */
namespace gate_decorator_system
{
    enum decorator_type
    {
        BDD,
        LUT
    };

    /**
     * Availability tester, i.e., has to return true if a decorator is available for the given gate
     *
     * @param[in] gate - The gate a decorator is queried for.
     * @returns True if a decorator is available.
     */
    using decorator_availability_tester = bool (*)(std::shared_ptr<gate> gate);
    /**
     * BDD decorator generator, i.e., has to create the bdd for a given gate using given inputs.
     *
     * @param[in] gate - The gate a decorator is queried for.
     * @param[in] inputs - The bdd mapping for the input ports.
     * @returns The bdd mapping for the output ports
     */
    using bdd_decorator_generator = std::map<std::string, std::shared_ptr<bdd>> (*)(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& inputs);
    /**
     * LUT decorator generator, i.e., has to return the lut configuration, combinational input and output size for a given gate. <br>
     * Input and output size have to be given this way, as it is not guaranteed that all LUT inputs are given to the combinational logic.
     *
     * @param[in] gate - The gate a decorator is queried for.
     * @returns A tuple (1) number of inputs, (2) number of outputs, (3) lut configuration string
     */
    using lut_decorator_generator = std::tuple<u32, u32, std::string> (*)(std::shared_ptr<gate> gate);

    /**
     * Registers a BDD gate decorator for a specific gate library.
     *
     * @param[in] gate_library - The supported gate library.
     * @param[in] tester - The tester that outputs whether a decorator is available for a given gate.
     * @param[in] generator - The generator capable of generating bdds for gates.
     */
    void register_bdd_decorator_function(const std::string& gate_library, decorator_availability_tester tester, bdd_decorator_generator generator);

    /*
     * Registers a LUT gate decorator for a specific gate library.
     *
     * @param[in] gate_library - The supported gate library.
     * @param[in] tester - The tester that outputs whether a decorator is available for a given gate.
     * @param[in] generator - The generator capable of extracting LUT configuration strings for gates.
     */
    void register_lut_decorator_function(const std::string& gate_library, decorator_availability_tester tester, lut_decorator_generator generator);

    /**
     * Removes the registered BDD gate decorator for a specific gate library.
     *
     * @param[in] gate_library - The supported gate library.
     */
    void remove_bdd_decorator_function(const std::string& gate_library);

    /**
     * Removes the registered LUT gate decorator for a specific gate library.
     *
     * @param[in] gate_library - The supported gate library.
     */
    void remove_lut_decorator_function(const std::string& gate_library);

    /**
     * Queries a gate decorator.
     *
     * @param[in] gate - The gate to decorate.
     * @param[in] decorator_type - The requested type.
     * @returns A shared pointer to the decorator, or nullptr on error.
     */
    std::shared_ptr<gate_decorator> query_decorator(std::shared_ptr<gate> gate, const decorator_type& type);

    /**
     * Checks whether the system has a decorator of the desired type for the gate.
     *
     * @param[in] gate - The gate to check for.
     * @param[in] decorator_type - The type to check for.
     * @returns True on success.
     */
    bool has_decorator_type(std::shared_ptr<gate> gate, const decorator_type& type);

}    // namespace gate_decorator_system

#endif /* __HAL_GATE_DECORATOR_SYSTEM_H__ */

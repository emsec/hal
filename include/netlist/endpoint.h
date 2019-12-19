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

#include "def.h"

/* forward declaration */
class gate;

/**
 *  Endpoint data structure for (gate, pin) tuples
 *
 * @ingroup netlist
 */
struct endpoint
{
    std::shared_ptr<::gate> gate;

    std::string pin_type;

    /**
    * Copies the value of an endpoint over to another endpoint.
    *
    * @param[in] copy - The endpoint to copy.
    * @returns The target endpoint.
    */
    endpoint& operator=(const endpoint& copy)
    {
        this->gate     = copy.gate;
        this->pin_type = copy.pin_type;
        return *this;
    }

    /**
    * Standard "less than". <br>
    * Required for searching through sets.
    *
    * @param[in] rhs - Compare target.
    * @returns True if this is less than rhs.
    */
    bool operator<(const endpoint& rhs) const
    {
        return (this->gate < rhs.gate) || ((this->gate == rhs.gate) && (this->pin_type < rhs.pin_type));
    }

    /**
    * Standard "equals". <br>
    * Required for searching through sets.
    *
    * @param[in] rhs - Compare target.
    * @returns True if this is equal to rhs.
    */
    bool operator==(const endpoint& rhs) const
    {
        return (this->gate == rhs.gate) && (this->pin_type == rhs.pin_type);
    }

    /**
    * Standard "unequal".
    *
    * @param[in] rhs - Compare target.
    * @returns True if this is unequal to rhs.
    */
    bool operator!=(const endpoint& rhs) const
    {
        return !(*this == rhs);
    }

    /**
    * Returns the gate of the current endpoint
    *
    * @returns The gate.
    */
    const std::shared_ptr<::gate>& get_gate() const
    {
        return gate;
    }

    /**
     * Sets the gate of the endpoint/
     * @param[in] g gate to be set
     */
    void set_gate(const std::shared_ptr<::gate>& g)
    {
        endpoint::gate = g;
    }

    /**
     * Returns the pin type of the current endpoint
     *
     * @returns pin_type as std::string
     */
    const std::string& get_pin_type() const
    {
        return pin_type;
    }

    /**
     * Sets the pin type of the current endpoint
     *
     * @param[in] type to be set
     */
    void set_pin_type(const std::string& type)
    {
        endpoint::pin_type = type;
    }
};

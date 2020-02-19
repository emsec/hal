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
class endpoint
{
public:
    endpoint(const std::shared_ptr<gate>& gate, const std::string& pin, bool is_a_dst)
    {
        m_gate     = gate;
        m_pin      = pin;
        m_is_a_dst = is_a_dst;
    }
    endpoint(const endpoint&) = default;
    endpoint(endpoint&&)      = default;
    endpoint& operator=(const endpoint&) = default;
    endpoint& operator=(endpoint&&) = default;

    /**
    * Standard "less than". <br>
    * Required for searching through sets.
    *
    * @param[in] rhs - Compare target.
    * @returns True if this is less than rhs.
    */
    bool operator<(const endpoint& rhs) const
    {
        if (this->m_gate != rhs.m_gate)
        {
            return (this->m_gate < rhs.m_gate);
        }
        if (this->m_pin != rhs.m_pin)
        {
            return (this->m_pin < rhs.m_pin);
        }
        return (this->m_is_a_dst < rhs.m_is_a_dst);
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
        return (this->m_gate == rhs.m_gate) && (this->m_pin == rhs.m_pin) && (this->m_is_a_dst == rhs.m_is_a_dst);
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
        return m_gate;
    }

    /**
     * Returns the pin of the current endpoint
     *
     * @returns pin as std::string
     */
    const std::string& get_pin() const
    {
        return m_pin;
    }

    /**
     * Checks the pin type of the current endpoint.
     *
     * @returns True, if the endpoint is an input pin.
     */
    bool is_dst_pin() const
    {
        return m_is_a_dst;
    }

    /**
     * Checks the pin type of the current endpoint.
     *
     * @returns True, if the endpoint is an output pin.
     */
    bool is_src_pin() const
    {
        return !m_is_a_dst;
    }

private:
    std::shared_ptr<gate> m_gate;
    std::string m_pin;
    bool m_is_a_dst;
};

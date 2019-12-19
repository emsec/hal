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

#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/* forward declaration*/
class netlist;

/**
 * @ingroup hdl_writers
 */
class HDL_FILE_WRITER_API hdl_writer
{
public:
    /**
     * @param[out] stream - The string stream which will be filled with the hdl code.
     */
    explicit hdl_writer(std::stringstream& stream);

    virtual ~hdl_writer() = default;

    /**
     * Serializes the netlist into hdl code.
     *
     * @param[in] g - The netlist.
     * @returns True on success.
     */
    virtual bool write(std::shared_ptr<netlist> const g) = 0;

protected:
    // stores the netlist
    std::shared_ptr<netlist> m_netlist;

    // holds the output stream to the file
    std::stringstream& m_stream;
};

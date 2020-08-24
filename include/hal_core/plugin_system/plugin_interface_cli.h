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

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/program_options.h"
#include "hal_core/def.h"

#include <tuple>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;

    /**
     * @ingroup core
     */
    class CORE_API CLIPluginInterface : virtual public BasePluginInterface
    {
    public:
        CLIPluginInterface()          = default;
        virtual ~CLIPluginInterface() = default;

        /**
         * Returns command line interface options
         *
         * @returns The program options description.
         */
        virtual ProgramOptions get_cli_options() const = 0;

        /**
         * Entry point to handle command line interface call
         *
         * @param[in] netlist - The netlist.
         * @param[in] args - Program options.
         * @returns True on success.
         */
        virtual bool handle_cli_call(Netlist* netlist, ProgramArguments& args) = 0;
    };
}    // namespace hal

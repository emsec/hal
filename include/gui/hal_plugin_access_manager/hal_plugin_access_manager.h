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

#ifndef HAL_PLUGIN_ACCESS_MANAGER_H
#define HAL_PLUGIN_ACCESS_MANAGER_H

#include "core/program_options.h"
#include <core/program_options.h>
#include <memory>
#include <string>

class i_cli;
class i_factory;
class i_gui;

class hal_plugin_access_manager
{
public:
    static program_arguments request_arguments(const std::string plugin_name);
    static int run_plugin(const std::string plugin_name, program_arguments* args);

private:
    hal_plugin_access_manager();
    static i_factory* get_plugin_factory(std::string plugin_name);
    static std::shared_ptr<i_cli> query_plugin_cli(i_factory* factory);
    static std::shared_ptr<i_gui> query_plugin_gui(i_factory* factory);
};

#endif    // HAL_PLUGIN_ACCESS_MANAGER_H

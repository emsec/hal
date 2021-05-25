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

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"

#include <unordered_set>
#include <QSet>

namespace hal
{
    /**
     * @ingroup gui
     */
    namespace gui_utility
    {
        /**
         * Finds the first common ancestor of a set of modules and gates. The gates are resolved to their corresponding
         * modules and then searching pairwise (all modules with the current first common acestor at the time) for the
         * "real" first common ancestor of all modules.
         *
         * @param modules - The set of modules for which the first common ancestor shall be found.
         * @param gates - The set of gates for which the first common accestor shall be found.
         * @return - The common ancestor for both the modules and the gates. If no ancestor is found, a nullpointer is returned.
         */
        Module* firstCommonAncestor(std::unordered_set<Module*> modules, const std::unordered_set<Gate*>& gates);

        /**
         * Finds the first common ancestor of both modules by first saving every parent module of m1 until it reachs
         * the "final" top module and then going step by step through every immediate parent of m2. The first hit that
         * is found is returned.
         *
         * @param m1 - The first module.
         * @param m2 -  The second module.
         * @return The first common ancestor. When no common ancestor is found, a nullpointer is returned.
         */
        Module* firstCommonAncestor(Module* m1, Module* m2);

        /**
         * Returns all parent modules for a given gate.
         * @param g - The gate for which all parent modules are required.
         * @return All parent modules up to the top module.
         */
        QSet<u32> parentModules(Gate* g);

        /**
         * Retirms all parent modules for a given module by iteratively going through every immediate parent
         * module until the top module is reached.
         *
         * @param m - The module for which all parent modules are required.
         * @return All parent modules up to the top module.
         */
        QSet<u32> parentModules(Module* m);
    }
}

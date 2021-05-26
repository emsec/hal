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

#include "gui/graph_widget/items/nodes/graphics_node.h"

#include <memory>
#include "gui/gui_utils/sort.h"

namespace hal
{
    class Module;

    /**
     * @ingroup graph-visuals-module
     * @brief Abstract base class for modules.
     *
     * Abstract base class for GraphicsModules. Besides the functionality already given by its superclass GraphicsNode
     * it stores the list of input and output pins in a unique struct.
     */
    class GraphicsModule : public GraphicsNode
    {
    public:
        /**
         * Constructor.
         *
         * @param m - The underlying module of this GraphicsModule
         */
        explicit GraphicsModule(Module* m);

    protected:
        /**
         * Represents one pin (both input or output) of a module. It stores the pin type name and the id of the net
         * connected to the pin. Besides, it provides comparison functions.
         */
        struct ModulePin
        {
            /** The name of the pin type */
            QString name;
            /** The id of the net connected to this pin */
            u32 mNetId;

            /** Two ModulePins are considered equal if both their names and their connected nets are the same */
            bool operator==(const ModulePin& rhs) const
            {
                return this->name == rhs.name && this->mNetId == rhs.mNetId;
            }

            /** The comparison is done based only on the name of both modules. (See gui_utility::numeratedOrderCompare) */
            bool operator<(const ModulePin& rhs) const
            {
                return gui_utility::numeratedOrderCompare(this->name, rhs.name);
            }
        };

        /**
         * The list of input pins of the module
         */
        QVector<ModulePin> mInputPins;

        /**
         * The list of output pins of the module
         */
        QVector<ModulePin> mOutputPins;
    };
}    // namespace hal

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
#include "user_action.h"

namespace hal
{
    class Module;
    class GraphContext;

    /**
     * @ingroup user_action
     * @brief Folds a module.
     *
     * Shows the parent module as one single box rather than showing its content.
     *
     * Undo Action: ActionUnfoldModule
     */
    class ActionFoldModule : public UserAction
    {
        u32 mContextId;
        PlacementHint mPlacementHint;
    public:
        /**
         * Action constructor.
         *
         * @param moduleId - The id of the module to fold.
         */
        ActionFoldModule(u32 moduleId = 0);
        QString tagname() const override;
        bool exec() override;

        /**
         * Sets the id of the context the module will be folded in.
         *
         * @param id - The context id
         */
        void setContextId(u32 id) { mContextId = id; }

        /**
         * Configures the placement hints for the folded module.
         *
         * @param hint - The new placement hints.
         */
        void setPlacementHint(PlacementHint hint) { mPlacementHint = hint; }
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionFoldModule
     */
    class ActionFoldModuleFactory : public UserActionFactory
    {
    public:
        ActionFoldModuleFactory();
        UserAction* newAction() const;
        static ActionFoldModuleFactory* sFactory;
    };
}

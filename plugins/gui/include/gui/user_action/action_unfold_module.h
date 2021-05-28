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
     * @brief Unfolds a module.
     *
     * Unfolds a module so that its content is shown in the GraphicsScene.
     *
     * User Action: ActionFoldModule
     */
    class ActionUnfoldModule : public UserAction
    {
        u32 mContextId;
        PlacementHint mPlacementHint;
    public:
        /**
         * Action constructor.
         *
         * @param moduleId - The id of the module to unfold.
         */
        ActionUnfoldModule(u32 moduleId = 0);
        QString tagname() const override;
        bool exec() override;

        /**
         * Sets the id of the context the module will be folded in.
         *
         * @param id - The context id
         */
        void setContextId(u32 id) { mContextId = id; }
        void setObject(const UserActionObject& obj) override;

        /**
         * Configures the placement hints for the unfolded module.
         *
         * @param hint - The new placement hints.
         */
        void setPlacementHint(PlacementHint hint) { mPlacementHint = hint; }
    private:
        void execInternal(Module* m, GraphContext* currentContext);
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionUnfoldModule
     */
    class ActionUnfoldModuleFactory : public UserActionFactory
    {
    public:
        ActionUnfoldModuleFactory();
        UserAction* newAction() const;
        static ActionUnfoldModuleFactory* sFactory;
    };
}

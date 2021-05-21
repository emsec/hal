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
#include <QSet>

namespace hal
{
    /**
     * @ingroup user_action
     * @brief Set the selection and focus.
     *
     * Sets the current selection and focus. The internal parameter are set directly by the SelectionRelay.
     *
     * Undo Action: SetSelectionFocus
     */
    class ActionSetSelectionFocus : public UserAction
    {
        friend class SelectionRelay;

        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        SelectionRelay::Subfocus mSubfocus;
        u32 mSubfocusIndex;

    public:
        /**
         * Action constructor.
         */
        ActionSetSelectionFocus() {;}

        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        bool exec() override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        /**
         * Checks if the new selection/focus of this action differs from the current selection/focus.
         *
         * @returns true iff the selection/focus differs from the current one.
         */
        bool hasModifications() const;
        void setObject(const UserActionObject &obj) override;
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionSetSelectionFocus
     */
    class ActionSetSelectionFocusFactory : public UserActionFactory
    {
    public:
        ActionSetSelectionFocusFactory();
        UserAction* newAction() const;
        static ActionSetSelectionFocusFactory* sFactory;
    };
}

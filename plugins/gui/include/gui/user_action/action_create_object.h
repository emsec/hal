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
    /**
     * @ingroup user_action
     * @brief Create a new item
     *
     * Creates a new object with a given type and name.
     *
     * Undo Action: ActionDeleteObject.
     */
    class ActionCreateObject : public UserAction
    {
        QString mObjectName;
        u32 mParentId;
    public:
        /**
         * Action Constructor.
         *
         * @param type - The UserActionObjectType of the item that should be created (default type: None)
         * @param objName - The name of the object to create (default name: "").
         */
        ActionCreateObject(UserActionObjectType::ObjectType type=UserActionObjectType::None,
                           const QString& objName = QString());
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        void setParentId(u32 pid) {mParentId = pid;}
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionCreateObject
     */
    class ActionCreateObjectFactory : public UserActionFactory
    {
    public:
        ActionCreateObjectFactory();
        UserAction* newAction() const;
        static ActionCreateObjectFactory* sFactory;
    };
}

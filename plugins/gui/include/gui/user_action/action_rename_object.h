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
#include <QString>

namespace hal
{
    /**
     * @ingroup user_action
     * @brief Renames an item.
     *
     * Assigns a new name to the UserActionObject's object. If the object is a pin type it has to be specified by the
     * id of the input/output net (configured by calling setInputNetId/setOutputNetId).
     *
     * Undo Action: ActionRenameObject
     */
    class ActionRenameObject : public UserAction
    {
        QString mNewName;
        u32 mNetId;
        enum PortType { NoPort, Input, Output } mPortType;
    public:
        /**
         * Action constructor.
         *
         * @param name - The new name
         */
        ActionRenameObject(const QString& name=QString())
            : mNewName(name), mNetId(0), mPortType(NoPort) {;}
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;

        /**
         * If the object to rename is a port, this function specifies the port by the connected input net.
         *
         * @param id - The id of the input net
         */
        void setInputNetId(u32 id)  { mPortType=Input;  mNetId=id; }

        /**
         * If the object to rename is a port, this function specifies the port by the connected output net.
         *
         * @param id - The id of the output net
         */
        void setOutputNetId(u32 id) { mPortType=Output; mNetId=id; }
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionRenameObject
     */
    class ActionRenameObjectFactory : public UserActionFactory
    {
    public:
        ActionRenameObjectFactory();
        UserAction* newAction() const;
        static ActionRenameObjectFactory* sFactory;
    };
}

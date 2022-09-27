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
     * @brief Reorders an item within a container.
     *
     * Reorders an item (only pins are currently supported, pingroups are planned) to a new position.
     * If the object's type is a pin or pingroup, the name is used to identify the object (configured
     * by setPinOrPingroupIdentifier or correct Constructor). In this case, object's id is used
     * to identify the corresponding module.
     */
    class ActionReorderObject : public UserAction
    {
        public:
            /**
             * Normal constructor (all purpose).
             *
             * @param newIndex - The new index.
             */
            ActionReorderObject(const int newIndex = -1);

            /**
             * Overwritten user_action function.
             *
             * @return True on success, False otherwise.
             */
            bool exec() override;

            /**
             * Overwritten user_action function.
             * @return The tag.
             */
            QString tagname() const override;

            /**
             * Overwritten user_action function.
             *
             * @param xmlOut - The writer.
             */
            void writeToXml(QXmlStreamWriter &xmlOut) const override;

            /**
             * Overwritten user_action function.
             *
             * @param xmlIn - The reader.
             */
            void readFromXml(QXmlStreamReader &xmlIn) override;

            /**
             * Overwritten user_action function.
             *
             * @param cryptoHash - The hash to add to.
             */
            void addToHash(QCryptographicHash &cryptoHash) const override;

        private:
            int mNewIndex;
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionReorderObject
     */
    class ActionReorderObjectFactory : public UserActionFactory
    {
    public:
        ActionReorderObjectFactory();
        UserAction * newAction() const override;
        static ActionReorderObjectFactory* sFactory;
    };

}

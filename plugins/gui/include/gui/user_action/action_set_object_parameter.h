// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once
#include "user_action.h"

namespace hal
{
    /**
     * A user action that sets the value of a typed parameter or attribute on a gate, net or module, and creates the
     * parameter when it does not exist yet. The value must be valid for the declaration's type. Undo restores the
     * previous value, or removes a parameter the action created; an action with `remove` set deletes the parameter
     * and its undo creates it again.
     */
    class ActionSetObjectParameter : public UserAction
    {
    public:
        /**
         * @param source - "generic" or "attribute"
         * @param name - The parameter name
         * @param value - The value to set, in the grammar of the parameter type
         * @param type - The type name for a parameter that does not exist yet ("bit_vector", "integer", ...); ignored for an existing one
         * @param size - The width for a new bit or logic vector
         */
        ActionSetObjectParameter(QString source = QString(), QString name = QString(), QString value = QString(), QString type = QString(), int size = 0);

        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;

        /**
         * Turn the action into a removal of the parameter.
         */
        void setRemove(bool remove = true);

    private:
        QString mSource;
        QString mName;
        QString mValue;
        QString mType;
        int mSize;
        bool mRemove;
    };

    class ActionSetObjectParameterFactory : public UserActionFactory
    {
    public:
        ActionSetObjectParameterFactory();
        UserAction* newAction() const override;
        static ActionSetObjectParameterFactory* sFactory;
    };
}    // namespace hal

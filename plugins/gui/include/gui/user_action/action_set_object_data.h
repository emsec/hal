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
    class ActionSetObjectData : public UserAction
    {

    public:
        ActionSetObjectData(QString category = QString(), QString key = QString(), QString type = QString(), QString val = QString());

        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;

        /**
         * This function must be called when the key and/or category property of an already existing entry
         * shall to be modified. Otherwise, instead of changing the 4 tupel from (oldKey, oldCat, oldType, oldVal)
         * to (newKey, newCat, newType, newVal), a new entry would be inserted and both of them would coexist.
         * (This also applies if, for example, one wants to only modify the category)
         *
         * @param oldKey - The old key that is to "potentionally" overwritten.
         * @param oldCategory - The old category that is to "potentionally" overwritten.
         */
        void setChangeKeyAndOrCategory(QString oldCategory, QString oldKey);

    private:
        QString mKey;
        QString mCat;
        QString mType;
        QString mVal;

        QString mOldKey;
        QString mOldCat;
        bool mKeyOrCatModified;
    };

    /**
     * @brief The ActionSetObjectDataFactory class
     */
    class ActionSetObjectDataFactory : public UserActionFactory
    {
    public:
        ActionSetObjectDataFactory();
        UserAction * newAction() const override;
        static ActionSetObjectDataFactory* sFactory;

    };

}

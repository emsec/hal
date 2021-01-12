//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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
#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "hal_core/defines.h"
#include "gui/selection_relay/selection_relay.h"

namespace hal {

     /**
     * @brief The UserActionObjectType class provides the type for UserActionObject
     *
     * There are conversation methods to conveniently convert the type for persistance
     * to xml file and for graphics ItemType
     */
    class UserActionObjectType : public QObject
    {
        Q_OBJECT
    public:
        enum ObjectType{ None, Module, Gate, Net, Netlist, Grouping, MaxObjectType };
        Q_ENUM(ObjectType)

        static ObjectType fromString(const QString& s);
        static QString toString(ObjectType t);
        static ObjectType fromItemType(SelectionRelay::ItemType itp);
        static SelectionRelay::ItemType toItemType(ObjectType t);
    };


    /**
    * @brief The UserActionObject class represents a single object used in UserAction
    *
    * An object is identified by unique id and type
    */
    class UserActionObject
    {
    public:
        /// object constructor, without arguments object of type None gets constructed
        UserActionObject(u32 i=0, UserActionObjectType::ObjectType t=UserActionObjectType::None)
            : mId(i), mType(t) {;}

        /// id of object
        u32 id() const { return mId; }

        /// type of object
        UserActionObjectType::ObjectType type() const { return mType; }

        /// persist object to xml attributes "id" and "type"
        void writeToXml(QXmlStreamWriter& xmlOut) const;

        /// reads object member variables from xml attributes "id" and "type"
        void readFromXml(QXmlStreamReader& xmlIn);
    private:
        u32 mId;
        UserActionObjectType::ObjectType mType;
    };

}

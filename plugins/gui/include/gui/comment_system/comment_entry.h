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


#include "gui/gui_def.h"
#include <QString>
#include <QDateTime>

namespace hal
{
    /**
     * @ingroup gui
     * @brief The CommentEntry class encapsulated information related to a comment.
     *
     * This class holds all relvant information for a gui comment such as a header, the time
     * of creation, and the text itself. For now only gates and modules can be associated with
     * a comment.
     *
     */
    class CommentEntry
    {
    public:
        CommentEntry(Node n, QString text = QString(), QString header = QString());
        ~CommentEntry(){};

        void setHeader(QString newHeader);
        void setText(QString newText);
        void setDirty(bool dirty);

        QString getHeader();
        QString getText();

        QDateTime getCreationTime();

        //create all setter/getter for datetimes for serialization/deserialization in case
        //the content manager parses file? Or omit them and create parse/write json methods
        //compareable to user actions, then there would be no need to set datetimes from outside.

    private:
        Node mNode;
        QString mHeader;
        QDateTime mCreated;
        QDateTime mLastModified;
        QString mText;
        bool mDirty;
    };
}

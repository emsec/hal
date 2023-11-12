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

#include <QFrame>
#include <QTextEdit>

#include "gui/searchbar/searchoptions.h"


class QVBoxLayout;
class QHBoxLayout;
class QToolBar;
class QLabel;
class QAction;
class QToolButton;

namespace hal
{
    class CommentEntry;
    class SearchableLabel;

    class CommentItem : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString modifyCommentIconPath READ modifyCommentIconPath WRITE setModifyCommentIconPath)
        Q_PROPERTY(QString modifyCommentIconStyle READ modifyCommentIconStyle WRITE setModifyCommentIconStyle)
        Q_PROPERTY(QString deleteCommentIconPath READ deleteCommentIconPath WRITE setDeleteCommentIconPath)
        Q_PROPERTY(QString deleteCommentIconStyle READ deleteCommentIconStyle WRITE setDeleteCommentIconStyle)
    public:
        CommentItem(QWidget* parent = nullptr);
        CommentItem(CommentEntry* entry, QWidget* parent = nullptr);
        ~CommentItem();

        void setComment(CommentEntry* entry);
        void updateCurrentEntry();
        bool search(const QString& string, SearchOptions searchOpts);

        CommentEntry* getEntry();

        // properties
        QString modifyCommentIconPath(){return mModifyCommentIconPath;}
        void setModifyCommentIconPath(const QString& str){mModifyCommentIconPath = str;}
        QString modifyCommentIconStyle(){return mModifyCommentIconStyle;}
        void setModifyCommentIconStyle(const QString& str){mModifyCommentIconStyle = str;}

        QString deleteCommentIconPath(){return mDeleteCommentIconPath;}
        void setDeleteCommentIconPath(const QString& str){mDeleteCommentIconPath = str;}
        QString deleteCommentIconStyle(){return mDeleteCommentIconStyle;}
        void setDeleteCommentIconStyle(const QString& str){mDeleteCommentIconStyle = str;}

    Q_SIGNALS:

        //perhaps the CommentEntry for which a delete is requested?
        //Getter for CommentEntry needed
        void delete_requested(CommentItem* item);

    private:
        QVBoxLayout* mLayout;
        CommentEntry* mEntry;

        QString mModifyCommentIconPath;
        QString mModifyCommentIconStyle;
        QString mDeleteCommentIconPath;
        QString mDeleteCommentIconStyle;

        // header part (perhaps no toolbar but a layout with toolbuttons instead of actions?)
        QToolBar* mTopToolbar;
        SearchableLabel* mHeader;
        QLabel* mCreationDate;
        QAction* mModifyAction;
        QAction* mDeleteAction;

        // header part v2
        QWidget* mTopWidget;
        QHBoxLayout* mTopLayout;
        QToolButton* mModifyButton;
        QToolButton* mDeleteButton;

        QTextEdit* mTextEdit;

        void init();

        void handleDeleteButtonTriggered();
        void handleModifyButtonTriggered();
    };
}

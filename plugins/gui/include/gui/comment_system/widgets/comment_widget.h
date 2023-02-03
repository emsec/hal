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

#include <QWidget>
#include "gui/gui_def.h"
#include <QLabel>
//#include <QVBoxLayout>
//#include <QHBoxLayout>
#include <QGridLayout>

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QToolBar;
class QToolButton;

namespace hal
{
    class Toolbar;
    class Searchbar;
    class CommentItem;
    class CommentEntry;

    class CommentWidget : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString newCommentIconPath READ newCommentIconPath WRITE setNewCommentIconPath)
        Q_PROPERTY(QString newCommentIconStyle READ newCommentIconStyle WRITE setNewCommentIconStyle)
        Q_PROPERTY(QString searchIconPath READ searchIconPath WRITE setSearchIconPath)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)

    public:
        CommentWidget(QWidget* parent = nullptr);
        ~CommentWidget();

        // property function
        QString newCommentIconPath(){return mNewCommentIconPath;}
        void setNewCommentIconPath(const QString& str){mNewCommentIconPath = str;}
        QString newCommentIconStyle(){return mNewCommentIconStyle;}
        void setNewCommentIconStyle(const QString& str){mNewCommentIconStyle = str;}

        QString searchIconPath(){return mSearchIconPath;}
        void setSearchIconPath(const QString& str){mSearchIconPath = str;}
        QString searchIconStyle(){return mSearchIconStyle;}
        void setSearchIconStyle(const QString& str){mSearchIconStyle = str;}

    public Q_SLOTS:
        void nodeChanged(const Node& nd);

    private:
        QList<CommentItem*> mEntryItems;
        QGridLayout* mTopLayout;
        Node mCurrentNode;

        QString mNewCommentIconPath;
        QString mNewCommentIconStyle;
        QString mSearchIconPath;
        QString mSearchIconStyle;

        // header part
        QHBoxLayout* mHeaderLayout; //perhaps put in a container?
        QToolBar* mToolbar; //put 2 actions and a widget in there? (or use simple qwidget(headerlayout) with buttons)
        //Toolbar* mToolbar;
        Searchbar* mSearchbar;
        QToolButton* mSearchButton;
        QToolButton* mNewCommentButton;

        // with toolbar:
        //add action - add spacer (expanding, preferred) - add action - add searchbar
        //when toggling the search action,  hide spacer (and/or change properties) and show searchbar
        //other idea: put the searchbar between header and comments, show/hide there (search action in header doesnt disappear)

        // comment part
        QScrollArea* mScrollArea;

        QWidget* mCommentsContainer;
        QVBoxLayout* mCommentsLayout;

        QWidget* createAndFillCommentContainerFactory(const Node& nd);

        void handleSearchbarTriggered();
        void handleNewCommentTriggered();

        // possible other params: commententry, node
        void handleCommentEntryDeleteRequest(CommentItem* item);
        void handleCommentAboutToBeDeleted(CommentEntry* entry);
        void handleEntryModified(CommentEntry* entry);
        void handleCommentAdded(CommentEntry* entry);
        void handleSearchbarTextEdited(const QString &text);
    };
}

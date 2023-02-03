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

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>

class QLineEdit;
class QTextEdit;
class QLabel;
class QToolBar;
class QAction;
class QTextCharFormat;

namespace hal
{
    class CommentEntry;
    class CommentDialog : public QDialog
    {
        Q_OBJECT
        Q_PROPERTY(QString redColor READ redColor WRITE setRedColor)
        Q_PROPERTY(QString yellowColor READ yellowColor WRITE setYellowColor)
        Q_PROPERTY(QString greenColor READ greenColor WRITE setGreenColor)
        Q_PROPERTY(QString defaultColor READ defaultColor WRITE setDefaultColor)
    public:
        // Perhaps remove the title-parameter and determine "New comment" and "Modify comment"
        // through the item parameter (or perhaps 2 constructors?)
        /**
         * Constructor. If the CommentEntry is a nullptr, this dialog is treated as a
         * "New Comment" dialog (no real differences to a "Modify" dialog).
         *
         * @param windowTitle - The title of the Dialog.
         * @param entry - The entry. If the entry is a nullptr, this dialog is a "New comment" dialog.
         * @param parent - The parent of the widget. Usually a nullptr.
         */
        CommentDialog(const QString windowTitle, CommentEntry* entry = nullptr, QWidget* parent = nullptr);
        ~CommentDialog();

        // Getter for the header and textfield if the dialog returns accapted?
        // (Or direct interaction with the manager?)

        /**
         * Returns the (new or modified) header of the comment. Usually called
         * from the instance that requested the dialog after the user clicked OK.
         *
         * @return The header.
         */
        QString getHeader();

        /**
         * Returns the (new or modified) text of the comment. Usually called
         * from the instance that requested the dialog after the user clicked OK.
         *
         * @return The actual comment in html format.
         */
        QString getText();

        //property tests
        QString redColor(){return mRedColor;}
        void setRedColor(const QString& str){mRedColor = str;}
        QString yellowColor(){return mYellowColor;}
        void setYellowColor(const QString & str){mYellowColor = str;}
        QString greenColor(){return mGreenColor;}
        void setGreenColor(const QString& str){mGreenColor = str;}
        QString defaultColor(){return mDefaultColor;}
        void setDefaultColor(const QString& str){mDefaultColor = str;}

    private:
        QVBoxLayout* mLayout;
        CommentEntry* mCommentEntry;
        QFont mDefaultFont;

        QString mRedColor;
        QString mYellowColor;
        QString  mGreenColor;
        QString mDefaultColor;

        // header part
        QWidget* mHeaderContainer; // for custom margins/spacing on top as well as colored background
        QHBoxLayout* mHeaderContainerLayout;
        QLineEdit* mHeaderEdit;
        QLabel* mLastModifiedLabel;

        // toolbar (Or container widget for Toolbuttons?)
        QToolBar* mToolBar;
        QAction* mBoldAction;
        QAction* mItalicsAction;
        QAction* mUnderscoreAction;
        QAction* mColorAction; // additional widget/indicatior to show current color?
        QAction* mListAction;
        QAction* mCodeAction;

        // textfield
        QTextEdit* mTextEdit;

        // helper functions
        void init();
        void mergeFormatOnWordOrSelection(const QTextCharFormat &format); // taken from richtext example
        void updateColorActionPixmap(const QColor &c);
        void handleColorSelected();
        void handleCurrentCharFormatChanged(const QTextCharFormat &format); // for the color
        void handleCursorPositionChanged(); // for the color (mTextEdit->textColor is the current color the curser is at)

        void boldTriggered();
        void italicsTriggered();
        void underscoreTriggered();
        void colorTriggered();
        void bulletListTriggered();
        void codeTriggered();

        void handleOkClicked();
        void handleCancelClicked();

    };
}

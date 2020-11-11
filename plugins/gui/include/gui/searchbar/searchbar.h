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

#include <QFrame>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QPushButton;
class QToolButton;

namespace hal
{
    class Searchbar : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString searchIcon READ searchIcon WRITE setSearchIcon)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString clearIcon READ clearIcon WRITE setClearIcon)
        Q_PROPERTY(QString clearIconStyle READ clearIconStyle WRITE setClearIconStyle)

    public:
        Searchbar(QWidget* parent = 0);

        QString searchIcon() const;
        QString searchIconStyle() const;
        QString clearIcon() const;
        QString clearIconStyle() const;

        void setSearchIcon(const QString& icon);
        void setSearchIconStyle(const QString& style);
        void setClearIcon(const QString& icon);
        void setClearIconStyle(const QString& style);

        void setPlaceholderText(const QString& text);
        void setModeButtonText(const QString& text);

        void clear();
        QString getCurrentText();

        void repolish();

    Q_SIGNALS:
        void textEdited(const QString& text);
        void returnPressed();
        void modeClicked();

    public Q_SLOTS:
        void handleTextEdited(const QString& text);
        void handleReturnPressed();
        void handleModeClicked();

    private:
        QHBoxLayout* mLayout;

        QLabel* mSearchIconLabel;
        QLineEdit* mLineEdit;
        QLabel* mClearIconLabel;
        QPushButton* mModeButton;
        QToolButton* mDownButton;
        QToolButton* mUpButton;

        QString mSearchIcon;
        QString mSearchIconStyle;
        QString mClearIcon;
        QString mClearIconStyle;
    };
}

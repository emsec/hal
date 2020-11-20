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

class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QVBoxLayout;

namespace hal
{
    class GetInTouchItem : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool hover READ hover)
        Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath)
        Q_PROPERTY(QString iconStyle READ iconStyle WRITE setIconStyle)

    public:
        explicit GetInTouchItem(const QString& title, const QString& mDescription, QWidget* parent = nullptr);

        void enterEvent(QEvent* event) Q_DECL_OVERRIDE;
        void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;
        void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

        void repolish();

        bool hover();
        QString iconPath();
        QString iconStyle();

        void setHoverActive(bool active);
        void setIconPath(const QString& path);
        void setIconStyle(const QString& style);

    Q_SIGNALS:
        void clicked();

    private:
        QHBoxLayout* mHorizontalLayout;
        QLabel* mIconLabel;
        QVBoxLayout* mVerticalLayout;
        QLabel* mTitleLabel;
        QLabel* mDescriptionLabel;
        QPropertyAnimation* mAnimation;

        bool mHover;

        QString mIconPath;
        QString mIconStyle;
    };
}

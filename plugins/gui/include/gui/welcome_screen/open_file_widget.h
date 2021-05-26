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
#include <QLabel>

class QVBoxLayout;

namespace hal
{
    /**
     * @ingroup gui
     * @brief The welcome screen's OpenFile section.
     *
     * The "Open File"-widget in the center of the welcome screen where the user can drag&drop a file to open/parse a
     * the netlist of the file.
     */
    class OpenFileWidget : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool dragActive READ dragActive)
        Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath)
        Q_PROPERTY(QString iconStyle READ iconStyle WRITE setIconStyle)

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        explicit OpenFileWidget(QWidget* parent = nullptr);

        /**
         * Handles the event that a user drags a file into the widget.
         *
         * @param event - the drag event
         */
        void dragEnterEvent(QDragEnterEvent* event) override;
        /**
         * Handles the event that a user drags a file out of the widget (that was previously dragged in)
         *
         * @param event
         */
        void dragLeaveEvent(QDragLeaveEvent* event) override;

        /**
         * Handles the event that a user drops a file in the widget. Used to open the dropped file.
         *
         * @param event
         */
        void dropEvent(QDropEvent* event) override;

        /** @name Q_PROPERTY READ Functions
         */
        ///@{
        bool dragActive();
        QString iconPath();
        QString iconStyle();
        ///@}

        /** @name Q_PROPERTY WRITE Functions
         */
        ///@{
        void setIconPath(const QString& path);
        void setIconStyle(const QString& style);
        ///@}

        /**
         * (Re-)Initializes the appearance of the widget.
         */
        void repolish();

    private:
        QVBoxLayout* mLayout;
        QLabel* mTextLabel;
        QLabel* mIconLabel;

        QString mIconPath;
        QString mIconStyle;

        bool mDragActive;
    };
}

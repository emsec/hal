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

#include <QLabel>
#include <QLayout>

namespace hal
{

    class ContentWidget;
    class Toolbar;

    /**
     * @ingroup docking
     * @brief Wraps the ContentWidget class.
     *
     * It displays the toolbar as well as the name and plays an important role in the docking system
     * regarding the detachment functionality.
     */
    class ContentFrame : public QWidget
    {
        Q_OBJECT

    public:

        /**
         * The ContentFrame's constructor.
         *
         * @param widget - The content widget that is wrapped.
         * @param attached - The boolean if the content widget (and the frame) are in a detached state.
         * @param parent - The frame's parent.
         */
        explicit ContentFrame(ContentWidget* widget, bool attached, QWidget* parent = nullptr);

        /**
         * Overwritten QWidget function to catch any QChildEvents. Catches the "removed" event.
         *
         * @param event - The triggered event.
         */
        void childEvent(QChildEvent* event) override;

        /**
         * Get the wrapped ContentWidget.
         *
         * @return The ContentWidget.
         */
        ContentWidget* content();

    public Q_SLOTS:

        /**
         * Calls the ContentWidget's detach function to detach the widget from its anchor. This function
         * is connected to the detach action (which action is displayed and connected is determined by the
         * attached boolean given in the constructor).
         */
        void detachWidget();

        /**
         * Calls the ContentWidget's reattach function to reattach the widget to its anchor. This function
         * is connected to the reattach action (which action is displayed and connected is determined by the
         * attached boolean given in the constructor).
         */
        void reattachWidget();

        /**
         * Sets the name label to the given name. Its connected to the ContentWidget's nameChanged signal.
         *
         * @param name - The new name.
         */
        void handleNameChanged(const QString& name);

    private:
        void closeEvent(QCloseEvent* event) override;

        QVBoxLayout* mVerticalLayout;
        QHBoxLayout* mHorizontalLayout;
        Toolbar* mLeftToolbar;
        Toolbar* mRightToolbar;
        ContentWidget* mWidget;
        QLabel* mNameLabel;

        QString mDetachIconPath;
        QString mDetachIconStyle;
    };
}

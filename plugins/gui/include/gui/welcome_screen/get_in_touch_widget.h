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

class QLabel;
class QVBoxLayout;

namespace hal
{
    class GetInTouchItem;

    /**
     * @ingroup gui
     * @brief The welcome screen's GetInTouch section.
     *
     * The "Get In Touch"-widget at the right side in the welcome screen. The user can click a field to show
     * license and version information, the c++/python documentation or the github bug report site in the browser.
     */
    class GetInTouchWidget : public QFrame
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        GetInTouchWidget(QWidget* parent = nullptr);

        /**
         * (Re-)Initializes the appearance of the widget.
         */
        void repolish();

    public Q_SLOTS:
        /**
         * Q_SLOT to handle that the 'About' field has been clicked.
         */
        void handleAboutItemClicked();

        /**
         * Q_SLOT to handle that the 'Open C++ Documentation' field has been clicked.
         * Opens the cpp documentation in the browser.
         */
        void handleCppDocumentationItemClicked();

        /**
         * Q_SLOT to handle that the 'Open Python Documentation' field has been clicked.
         * Opens the pythin documentation in the browser.
         */
        void handlePyDocumentationItemClicked();

        /**
         * Q_SLOT to handle that the 'Found a bug?' field has been clicked.
         * Opens the github bug report site in the browser.
         */
        void handleTicketItemClicked();

    private:
        QVBoxLayout* mLayout;

        GetInTouchItem* mAboutItem;
        GetInTouchItem* mNewsItem;
        GetInTouchItem* mForumItem;
        GetInTouchItem* mCppDocumentationItem;
        GetInTouchItem* mPyDocumentationItem;
        GetInTouchItem* mTicketItem;
    };
}

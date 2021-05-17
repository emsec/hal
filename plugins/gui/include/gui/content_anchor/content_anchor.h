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

namespace hal
{
    class ContentWidget;

    /**
     * @ingroup docking
     * @brief An abstract class that provides the interface for specific anchors
     * (see SplitterAnchor or TabWidget ).
     */
    class ContentAnchor
    {
    public:
        /**
         * Adds a given ContentWidget to the anchor.
         *
         * @param widget - The widget to add.
         * @param index - The position.
         */
        virtual void add(ContentWidget* widget, int index) = 0;

        /**
         * Removes a given widget from the anchor.
         *
         * @param widget - The widget to remove.
         */
        virtual void remove(ContentWidget* widget) = 0;

        /**
         * Detaches a given widget from the anchor.
         *
         * @param widget - The widget to detach.
         */
        virtual void detach(ContentWidget* widget) = 0;

        /**
         * Reattaches a given widget to the anchor.
         *
         * @param widget - The widget to reattach.
         */
        virtual void reattach(ContentWidget* widget) = 0;

        /**
         * Displays a given widget in the achor's corresponding area.
         *
         * @param widget - The widget to open.
         */
        virtual void open(ContentWidget* widget) = 0;

        /**
         * Closes a given widget.
         *
         * @param widget - The widget to close.
         */
        virtual void close(ContentWidget* widget) = 0;

        /**
         * Destructor that has to be overriden.
         */
        virtual inline ~ContentAnchor() = 0;
    };

    ContentAnchor::~ContentAnchor()
    {
    }
}

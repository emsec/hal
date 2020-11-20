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

#include "gui/content_anchor/content_anchor.h"

#include <QList>
#include <QObject>

namespace hal
{
    class DockBar;
    class Splitter;
    class ContentFrame;
    class ContentWidget;

    class SplitterAnchor : public QObject, public ContentAnchor
    {
        Q_OBJECT

    public:
        SplitterAnchor(DockBar* DockBar, Splitter* Splitter, QObject* parent = 0);

        virtual void add(ContentWidget* widget, int index = -1) override;
        virtual void remove(ContentWidget* widget) override;
        virtual void detach(ContentWidget* widget) override;
        virtual void reattach(ContentWidget* widget) override;
        virtual void open(ContentWidget* widget) override;
        virtual void close(ContentWidget* widget) override;

        int count();
        void removeContent();

    Q_SIGNALS:
        void contentChanged();

    private:
        DockBar* mDockBar;
        Splitter* mSplitter;
        QList<ContentFrame*> mDetachedFrames;
    };
}

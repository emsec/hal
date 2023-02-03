// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <QTabWidget>
#include "gui/selection_details_widget/selection_details_icon_provider.h"

namespace hal
{
    class DetailsFrameWidget;

    class DetailsTabWidget : public QTabWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        DetailsTabWidget(QWidget* parent = nullptr);

    protected:

        /**
         * Adds a tab to the widget with the provided label. The contents of the tab are determined by the frame list.
         *
         * @param label - Label text in the tab bar.
         * @param frames - Frames which build the content of the tab.
         */
        int addTab(const QString& label, QList<DetailsFrameWidget*> frames);

        /**
         * Convenient function to add a tab to the widget with the provided label and single frame as content.
         *
         * @param label - Label text in the tab bar.
         * @param frame - Single Frame which build the content of the tab.
         */
        int addTab(const QString& label, DetailsFrameWidget* frame);

        /**
         *  Set the top right corner icon of the tab details widget.
         *
         * @param fileName - Icon name from gui ressources file.
         */
        void setIcon(SelectionDetailsIconProvider::IconCategory catg, u32 itemId=0);
    };
}

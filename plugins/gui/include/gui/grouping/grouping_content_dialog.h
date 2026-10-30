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

#include "hal_core/defines.h"

#include <QDialog>

class QLabel;
class QTimer;

namespace hal
{
    class Grouping;
    class SelectionTreeView;

    /**
     * @ingroup utility_widgets-grouping
     * @brief Window that shows the modules, gates and nets of a grouping.
     *
     * The window does not block the rest of HAL, so several of them can be open at once. It follows changes to the
     * name, the color and the content of its grouping, and closes when the grouping is deleted or the netlist is closed.
     */
    class GroupingContentDialog : public QDialog
    {
        Q_OBJECT

    public:
        /**
         * Shows the content window of a grouping. If the parent has a window open for this grouping already, that window
         * is brought to the front instead of opening another one.
         *
         * @param grouping - The grouping
         * @param parent - The parent widget of the window
         * @returns the window
         */
        static GroupingContentDialog* showGrouping(Grouping* grouping, QWidget* parent);

        /**
         * Constructor.
         *
         * @param grouping - The grouping
         * @param parent - The parent widget
         */
        GroupingContentDialog(Grouping* grouping, QWidget* parent = nullptr);

        /**
         * Get the grouping the window shows.
         *
         * @returns the grouping
         */
        Grouping* grouping() const;

    private Q_SLOTS:
        void handleGroupingNameChanged(Grouping* grp);
        void handleGroupingColorChanged(Grouping* grp);
        void handleGroupingContentChanged(Grouping* grp);
        void handleGroupingRemoved(Grouping* grp);
        void handleUpdateTimeout();

    private:
        void refresh();
        void updateTitle();
        void updateColor();

        Grouping* mGrouping;
        u32 mGroupingId;
        QLabel* mColorLabel;
        SelectionTreeView* mSelectionTreeView;
        QTimer* mUpdateTimer;
    };
}

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

#include "hal_core/defines.h"

#include "hal_core/netlist/net.h"
#include "hal_core/netlist/net.h"

#include "gui/gui_def.h"

#include <QTableWidget>

namespace hal
{
    class GraphGraphicsView;

    class GraphNavigationWidget : public QTableWidget
    {
        Q_OBJECT

    public:
        explicit GraphNavigationWidget(QWidget *parent = nullptr);

        // right = true
        void setup(bool direction);
        void setup(Node origin, Net* via_net, bool direction);
        void hideWhenFocusLost(bool hide);

    Q_SIGNALS:
        void navigationRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
        void closeRequested();
        void resetFocus();

    protected:
        void focusOutEvent(QFocusEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;

    private:
        void fillTable(Net* n, bool direction);
        void handleItemDoubleClicked(QTableWidgetItem* item);
        void commitSelection();

        GraphGraphicsView* mView;
        u32 mViaNet;
        Node mOrigin;
        bool mHideWhenFocusLost;
    };
}

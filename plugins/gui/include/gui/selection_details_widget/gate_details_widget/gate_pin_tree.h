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

#include "gui/gui_globals.h"
#include "hal_core/defines.h"
#include <QTreeView>

namespace hal
{
    class GatePinsTreeModel;
    class Gate;
    class BaseTreeItem;
    class GraphNavigationWidget;

    /**
     * @brief A widget to display the pins of a given gate.
     */
    class GatePinTree : public QTreeView
    {
        Q_OBJECT
    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        GatePinTree(QWidget* parent = nullptr);

        /**
         * Sets the gates and updates its model to display the gate's pins.
         *
         * @param gateID - The gate id.
         */
        void setGate(u32 gateID);

        /**
         * Sets the gates and updates its model to display the gate's pins.
         *
         * @param gateID - The gate.
         */
        void setGate(Gate* g);

        /**
         * Resets the model and shows an empty view as a result.
         */
        void removeContent();

        /**
         * Overwritten qwidget function to handle navigation through mouseclicks.
         *
         * @param event - The mouseevent.
         */
        virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

        /** @name Event Handler Functions
         */
        ///@{
        void handleContextMenuRequested(const QPoint &pos);
        ///@}

    Q_SIGNALS:
        /**
         * Emits the new headline when the number of displayed pins changes.
         *
         * @param newHeadline - The new complete headline.
         */
        void updateText(const QString& newHeadline);

    private:
        GraphNavigationWidget* mNavigationTable;
        GatePinsTreeModel* mPinModel;
        int mGateID;
        //saves the state if the selection shall be cleared when "jumping" to a source/destination
        bool mClearSelection;

        //helper functions
        void buildPythonMenuForPin(QMenu &menu, BaseTreeItem* clickedPinItem);
        void buildPythonMenuForPinGroup(QMenu &menu, BaseTreeItem* clickedPinIGrouptem);
        void addSourceOurDestinationToSelection(int netId, bool isInputPin);
        void handleNavigationCloseRequested();
        void handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);

    };

}

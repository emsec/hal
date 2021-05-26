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

#include <QTreeView>

namespace hal
{


    /**
     * @ingroup utility_widgets-module
     * @brief Wraps the QTreeView used for the ModuleWidget.
     *
     * It is necessary to prevent that a right click on the tree-view selects the
     * underlying item and overwrites the previous selection. Otherwise you cant select the option
     * <i>add selection to module</i> in the contextmenu.
     */
    class ModuleTreeView : public QTreeView
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        ModuleTreeView(QWidget* parent = nullptr);

        /**
         * Captures the mousePressEvent for right-clicks to prevent a reselection.
         *
         * @param event
         */
        void mousePressEvent(QMouseEvent* event);
    };
}    // namespace hal

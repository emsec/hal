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

#include "gui/content_widget/content_widget.h"

namespace hal
{
    class PythonConsole;

    /**
     * @ingroup python-console
     * @brief Wraps the PythonConsole.
     *
     * The ContentWidget that wraps the PythonConsole.
     */
    class PythonConsoleWidget : public ContentWidget
    {
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget.
         */
        PythonConsoleWidget(QWidget* parent = nullptr);

        /**
         * Setups the toolbar. Nothing done here, since the console has no toolbar.
         *
         * @param Toolbar - The Toolbar to set up.
         */
        virtual void setupToolbar(Toolbar* Toolbar) override;

        /**
         * Create all shortcuts that are associated with the python console.
         * There are currently no such shortcuts.
         *
         * @returns an list of shortcuts.
         */
        virtual QList<QShortcut*> createShortcuts() override;

    private:
        PythonConsole* mConsole;
    };
}

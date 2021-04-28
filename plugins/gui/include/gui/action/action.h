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

#include <QAction>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Extension for the QAction class.
     *
     * This extension supports the automatic display of shortcuts within the QAction's Tooltip.
     */
    class Action : public QAction
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent object (can be an action group)
         */
        explicit Action(QObject *parent = nullptr);

        /**
         * Constructor.
         *
         * @param text - The Action's description
         * @param parent - The parent object (can be an action group)
         */
        explicit Action(const QString &text, QObject *parent = nullptr);

        /**
         * Constructor.
         *
         * @param icon - The icon of the Action
         * @param text - The Action's description
         * @param parent - The parent object (can be an action group)
         */
        explicit Action(const QIcon &icon, const QString &text, QObject *parent = nullptr);

        // Hides non-virtual methods in QAction:
        /**
         * Sets the description of the Action. Also updates the Action's tooltip.
         *
         * @param text - The new description
         */
        void setText(const QString& text);

        /**
         * Changes the tooltip to the specified string. If the tooltip is changed once by this function, the
         * tooltip won't be automatically extended by the Action's shortcut any longer. Therefore, using this function
         * one can create custom tooltips without shortcuts using this function.
         *
         * @param tooltip - The new tooltip
         */
        void setToolTip(const QString& tooltip);

    public Q_SLOTS:
        /**
         * Configures a shortcut for this Action.
         *
         * @param shortcut - The shortcut to configure
         */
        void setShortcut(const QKeySequence &shortcut);

    private:
        QString mDescriptionText;
        void updateTooltip(const QKeySequence& seq);
        bool mTooltipModified;

    };
}

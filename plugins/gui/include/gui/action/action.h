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

#include <QAction>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Provides an interface for triggerable functionality that can be inserted into widgets
     * and also connected to shortcuts.
     *
     * The Action class encapsulates the functionality of a button than can be put into menus and can
     * can also be triggered via shortcuts. Common examples for actions are the "open" or "save" options
     * in the top bar of the main widget as well as all icons in the python editor.
     */
    class Action : public QAction
    {
        Q_OBJECT

    public:
        /**
         * The default constructor.
         *
         * @param parent - The action's parent.
         */
        explicit Action(QObject *parent = nullptr);

        /**
         * Second constructor where one can also set the descriptive text.
         *
         * @param text - The text.
         * @param parent - The action's parent.
         */
        explicit Action(const QString &text, QObject *parent = nullptr);

        /**
         * Third constructor where one can set the descriptive text as well as the displayed icon.
         *
         * @param icon - The displayed icon.
         * @param text - The descriptive text.
         * @param parent - The action's parent.
         */
        explicit Action(const QIcon &icon, const QString &text, QObject *parent = nullptr);

        // hides non-virtual methods in QAction
        /**
         * Sets the descriptive text of the action.
         *
         * @param text - The text to set.
         */
        void setText(const QString& text);

        /**
         * Sets the tooltip of the action.
         *
         * @param tooltip - The tooltip
         */
        void setToolTip(const QString& tooltip);

    public Q_SLOTS:

        /**
         * Sets the shortcut with which one can trigger the action.
         *
         * @param shortcut - The shortcut.
         */
        void setShortcut(const QKeySequence &shortcut);

    private:
	void updateTooltip(const QKeySequence& seq);

        QString mDescriptionText;
        bool mTooltipModified;

    };
}

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

#include "hal_core/defines.h"

#include <QFont>
#include <QWidget>
#include <QLabel>

namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     * @brief Base class for specific details widgets.
     *
     * The base-class for the specific gate, net and module details widgets. It defines commonly shared
     * information and functions such as the keyfont and the current id and makes these accessible.
     */
    class DetailsWidget : public QWidget
    {
        Q_OBJECT
    public:

        /**
         * This enum is used by the subclasses to specify which details widget it is.
         */
        enum DetailsType
        {
            ModuleDetails,
            GateDetails,
            NetDetails
        };

        /**
         * The constructor.
         *
         * @param tp - The type of the specific subclass widget.
         * @param parent - The widget's parent.
         */
        explicit DetailsWidget(DetailsType tp, QWidget* parent = nullptr);

        /**
         * Get the font that is used for the information table's key entries.
         *
         * @return The keyfont.
         */
        QFont keyFont() const;

        /**
         * Get the currently displayed id of the corresponding item.
         *
         * @return The item's id.
         */
        u32 currentId() const;

        /**
         * Get the type of the subclass widget as a string by converting the DetailsType enum.
         *
         * @return The widget's type.
         */
        QString detailsTypeName() const;

        /**
         * Get the label that contains the big icon for each specific widget.
         *
         * @return The icon containing label.
         */
        QLabel* bigIcon();

        /**
         * Sets the flag to hide empty sections when they are empty.
         *
         * @param hide - True to hide empty sections, False otherwise.
         */
        virtual void hideSectionsWhenEmpty(bool hide);

    protected:
        DetailsType mDetailsType;
        u32 mCurrentId;
        QFont mKeyFont;
        bool mHideEmptySections;
        QLabel* mBigIcon;
    };
}    // namespace hal

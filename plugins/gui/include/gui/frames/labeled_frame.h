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

#include <QFrame>

class QHBoxLayout;
class QLabel;
class QVBoxLayout;

namespace hal
{
    /**
     * @ingroup gui
     * @brief A QFrame with a label.
     *
     * The LabeledFrame is a QFrame that has a label at the top. The label can be configured in the qss file.
     *
     * In HAL it is used for the three frames in the welcome screen.
     */
    class LabeledFrame : public QFrame
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         * Creates an empty LabeledFrame with a label. The labels content is taken from the used qss file.
         *
         * @param parent - The parent widget
         */
        LabeledFrame(QWidget* parent = nullptr);

        /**
         * Adds a widget to the LabeledFrames layout.
         *
         * @param content
         */
        void addContent(QWidget* content);

    protected:
        QVBoxLayout* mLayout;
        QFrame* mHeader;
        QHBoxLayout* mHeaderLayout;
        QFrame* mLeftSpacer;
        QLabel* mLabel;
        QFrame* mRightSpacer;
    };
}

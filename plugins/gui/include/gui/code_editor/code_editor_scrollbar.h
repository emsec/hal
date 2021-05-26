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

#include <QScrollBar>

namespace hal
{
    class MinimapScrollbar;

    /**
     * @ingroup python-editor
     * @brief Represents the scrollbar of the CodeEditor.
     *
     * It overrides the QScrollBar class to extend its functionality so that
     * the CodeEditorScrollbar is able to synchronize with the MinimapScrollbar of the CodeEditor.
     */
    class CodeEditorScrollbar : public QScrollBar
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        explicit CodeEditorScrollbar(QWidget* parent = nullptr);

        /**
         * Sets the MinimapScrollbar this scollbar should be stay in sync with.
         *
         * @param scrollbar - The minimap scrollbar to associate with
         */
        void setMinimapScrollbar(MinimapScrollbar* scrollbar);

    protected:
        /**
         * Handles the sliderChange event to stay in sync with the MinimapScrollbar.
         *
         * @param change - the slider change event
         */
        virtual void sliderChange(SliderChange change) override;

    private:
        MinimapScrollbar* mMinimapScrollbar;
    };
}

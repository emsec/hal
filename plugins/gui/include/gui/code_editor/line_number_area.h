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

#include <QWidget>

namespace hal
{
    class CodeEditor;

    /**
     * @ingroup python-editor
     * @brief Shows line numbers next to a CodeEditor.
     *
     * The LineNumberArea is a widget next to its associated CodeEditor widget that shows the line numbers for the
     * content of this CodeEditor. This widget provides an area where these line numbers can be entered and stores
     * information about the size of the line number area. However, it is the responsibility of the CodeEditor to
     * insert the correct line numbers in the area, after the widget calls CodeEditor::lineNumberAreaPaintEvent in
     * its paint event.
     */
    class LineNumberArea : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(int leftOffset READ leftOffset WRITE setLeftOffset)
        Q_PROPERTY(int rightOffset READ rightOffset WRITE setRightOffset)

    public:
        /**
         * Constructor.
         *
         * @param editor - The CodeEditor this line number area should be associated with. Must not be a <i>nullptr</i>.
         */
        explicit LineNumberArea(CodeEditor* editor);

        /** @name Q_PROPERTY READ Functions */
        ///@{
        int leftOffset() const;
        int rightOffset() const;
        ///@}

        /** @name Q_PROPERTY WRITE Functions */
        ///@{
        void setLeftOffset(const int offset);
        void setRightOffset(const int offset);
        ///@}

        /**
         * Reinitializes the appearance of the line number area widget.
         */
        void repolish();

    protected:
        /**
         * Paints the line number are widget. Afterwards it invokes CodeEditor::lineNumberAreaPaintEvent so that the
         * line numbers are inserted
         *
         * @param event - The paint event
         */
        void paintEvent(QPaintEvent* event) override;

        /**
         * Handles a wheel event so that the event is passed to the CodeEditor. It allows to scroll in the
         * code editor while the mouse hovers above the line number area.
         *
         * @param event - The wheel event
         */
        void wheelEvent(QWheelEvent* event) override;

    private:
        CodeEditor* mEditor;

        int mLeftOffset;
        int mRightOffset;
    };
}

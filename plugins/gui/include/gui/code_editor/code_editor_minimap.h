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

class QTextDocument;

namespace hal
{

    class CodeEditor;
    class MinimapScrollbar;

    /**
     * @ingroup python-editor
     * @brief A minimap that supports an easier navigation in larger files.
     *
     * It shows the zoomed out content of the file the associated CodeEditor currently works on.
     * Moreover it adapts the functionality of the default scroll bar which is extended by this minimap.
     */
    class CodeEditorMinimap : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param editor - The CodeEditor the minimap displays
         */
        explicit CodeEditorMinimap(CodeEditor* editor);

        /**
         * Gets the minimap scrollbar of the minimap.
         *
         * @returns the minimap's scrollbar
         */
        MinimapScrollbar* scrollbar();

        /**
         * Gets the underlying text document of the minimap
         *
         * @returns the text document of the minimap
         */
        QTextDocument* document();

        /**
         * Sets the height of the minimap slider based on the height of the associated code editor.
         *
         * @param viewport_height - The viewport height (unused)
         */
        void adjustSliderHeight(int viewport_height);

        /**
         * Sets the height of the minimap slider to a specified value.
         *
         * @param ratio - The proportion of the total minimap that should be the height of the slider
         */
        void adjustSliderHeight(qreal ratio);

        /**
         * Sets the height of the minimap slider based on a block (line) range.
         *
         * @param first_visible_block - The first element of the block range (inclusive)
         * @param last_visible_block - The last element of the block range (exclusive)
         */
        void adjustSliderHeight(int first_visible_block, int last_visible_block);

        /**
         * Reinitializes the appearance of the minimap widget.
         */
        void repolish();

    public Q_SLOTS:
        /**
         * Q_SLOT to handle that the size of the document has been changed.
         *
         * @param new_size - The new size of the document
         */
        void handleDocumentSizeChanged(const QSizeF& new_size);

        /**
         * Q_SLOT to handle that the content of the associated code editor has been changed.
         *
         * @param position - The position of the character in the document where the change occurred
         * @param chars_removed - The number of removed characters
         * @param chars_added - The number of added characters
         */
        void handleContentsChange(int position, int chars_removed, int chars_added);

    protected:
        /**
         * Handles the paint event for the minimap to draw it.
         *
         * @param event - The paint event
         */
        void paintEvent(QPaintEvent* event) override;

        /**
         * Handles the resize event for the minimap to adjust the size of the scroll bar.
         *
         * @param event - The resize event
         */
        void resizeEvent(QResizeEvent* event) override;

        /**
         * Handles the mouse press event to scroll to the respective position in the associated code editor.
         *
         * @param event - The mouse event
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * Handles the mouse wheel event to pass it to the associated code editor.
         *
         * @param event - The mouse wheel event
         */
        void wheelEvent(QWheelEvent* event) override;

    private:
        void resizeScrollbar();

        CodeEditor* mEditor;
        QTextDocument* mDocument;
        MinimapScrollbar* mScrollbar;

        int mDocumentHeight;
        qreal mOffset;
    };
}

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

#include "gui/code_editor/code_editor_minimap.h"

#include <QPlainTextEdit>

class QPaintEvent;
class QPropertyAnimation;
class QResizeEvent;

namespace hal
{

    class CodeEditorMinimap;
    class CodeEditorScrollbar;
    class LineNumberArea;

    /**
     * @ingroup python-editor
     * @brief A plain text edit widget that is intended for editing code.
     *
     * Therefore it has numbered lines and provides a code minimap for easier navigation in larger files.
     * Moreover it supports a search function to mark all occurrences of a specified string.
     */
    class CodeEditor : public QPlainTextEdit
    {
        Q_OBJECT
        Q_PROPERTY(QFont lineNumberFont READ lineNumberFont WRITE setLineNumberFont)
        Q_PROPERTY(QColor lineNumberColor READ lineNumberColor WRITE setLineNumberColor)
        Q_PROPERTY(QColor lineNumberBackground READ lineNumberBackground WRITE setLineNumberBackground)
        Q_PROPERTY(QColor lineNumberHighlightColor READ lineNumberHighlightColor WRITE setLineNumberHighlightColor)
        Q_PROPERTY(QColor lineNumberHighlightBackground READ lineNumberHighlightBackground WRITE setLineNumberHighlightBackground)
        Q_PROPERTY(QColor currentLineBackground READ currentLineBackground WRITE setCurrentLineBackground)

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        explicit CodeEditor(QWidget* parent = nullptr);

        /**
         * Captures the mouse scroll event to zoom in and out via CTRL+SCROLL UP/DOWN.
         *
         * @param object - The object this filter is applied on.
         * @param event - The event to filter
         * @returns true if the event is handled by this filter and shouldn't be processed any further by the target
         *          object. Returns false to pass the event to the target object.
         */
        virtual bool eventFilter(QObject* object, QEvent* event) override;

        /**
         * Paint event handler to paint the line numbers. This function is called when an paint event of the line
         * number area occurs. It draws the necessary line numbers in the line number area.
         *
         * @param event - The paint event
         */
        void lineNumberAreaPaintEvent(QPaintEvent* event);

        /**
         * Paint event handler to paint the line numbers.
         *
         * @param event - The paint event
         */
        void minimapPaintEvent(QPaintEvent* event);

        /**
         * Gets the width of area where the line numbers are drawn.
         *
         * @returns the line number area width.
         */
        int lineNumberAreaWidth();

        /**
         * Gets the width of the minimap.
         *
         * @returns the width of the minimap.
         */
        int minimapWidth();

        /**
         * Returns the index of the first visible block.
         *
         * @returns the index of the first visible block
         */
        int first_visible_block();

        /**
         * Scrolls to the specified line number using a scroll animation.
         *
         * @param number - The number to scroll to
         */
        void centerOnLine(const int number);

        /**
         * This function can be used to pass a wheel event to the code editor. It is necessary for widgets that are
         * associated with the code editor but are not part of it (e.g. the line number area or the minimap).
         *
         * @param event - The wheel event to pass to the code editor
         */
        void handleWheelEvent(QWheelEvent* event);

        /**
         * Accesses the code editors minimap.
         *
         * @returns the code editors minimap.
         */
        CodeEditorMinimap* minimap();

        /** @name Q_PROPERTY READ Functions */
        ///@{
        QFont lineNumberFont() const;
        QColor lineNumberColor() const;
        QColor lineNumberBackground() const;
        QColor lineNumberHighlightColor() const;
        QColor lineNumberHighlightBackground() const;
        QColor currentLineBackground() const;

        ///@}
        /** @name Q_PROPERTY WRITE Functions */
        ///@{
        void setLineNumberFont(const QFont& font);
        void setLineNumberColor(QColor& color);
        void setLineNumberBackground(QColor& color);
        void setLineNumberHighlightColor(QColor& color);
        void setLineNumberHighlightBackground(QColor& color);
        void setCurrentLineBackground(QColor& color);
        ///@}


        /**
         * Enables or disables the line numbers.
         *
         * @param enabled - True to enable, False to disable.
         */
        void setLineNumberEnabled(bool enabled);

        /**
         * Enables or disables the highlight of the current line.
         *
         * @param enabled - True to enable, False to disable.
         */
        void setHighlightCurrentLineEnabled(bool enabled);

        /**
         * Enables or disables the linewrap.
         *
         * @param enabled - True to enable, False to disable.
         */
        void setLineWrapEnabled(bool enabled);

        /**
         * Enables or disables the minimap.
         *
         * @param enabled - True to enable, False to disable.
         */
        void setMinimapEnabled(bool enabled);

    public Q_SLOTS:
        /**
         * Marks all occurrences of the specified string in the code editor.
         *
         * @param string - The string to mark
         * @param options - Optional QTextDocument::FindFlags (e.g. "FindCaseSensitively" or "FindWholeWords")
         */
        void search(const QString& string, QTextDocument::FindFlags options = QTextDocument::FindFlags());

        /**
         * Shows/Hides the line numbers at the left side of the code editor.
         */
        void toggleLineNumbers();

        /**
         * Shows/Hides the minimap of the code editor.
         */
        void toggleMinimap();

        /**
         * Configures the font size of the code editor.
         *
         * @param pt - The new font size
         */
	    void setFontSize(int pt);

    protected:
        /**
         * Captures the resize event to adjust the slider size of the minimap.
         *
         * @param event  - The resize event
         */
        virtual void resizeEvent(QResizeEvent* event) override;

    private Q_SLOTS:
        /**
         * Highlights the line (block if in line wrap mode) where the cursor is currently in.
         */
        void highlightCurrentLine();

        /**
         * Handles that the line (block if in line wrap mode) count has been changed. Used to adjust the slider height.
         *
         * @param new_block_count - The new amount of blocks
         */
        void handleBlockCountChanged(int new_block_count);

        /**
         * Handles the code editors update request for the line number area.
         *
         * @param rect - The rectangle the document need an update of (unused)
         * @param dy - The amount of pixels the viewport was vertically scrolled (unused)
         */
        void updateLineNumberArea(const QRect& rect, int dy);

        /**
         * Handles the code editors update request for the minimap.
         *
         * @param rect - The rectangle the document need an update of (unused)
         * @param dy - The amount of pixels the viewport was vertically scrolled (unused)
         */
        void updateMinimap(const QRect& rect, int dy);

    private:
        void updateLayout();
        void clearLineHighlight();

        CodeEditorScrollbar* mScrollbar;

        LineNumberArea* mLineNumberArea;
        CodeEditorMinimap* mMinimap;

        QPropertyAnimation* mAnimation;

        bool mLineNumbersEnabled;
        bool mLineHighlightEnabled;
        bool mMinimapEnabled;
        bool mLineWrapEnabled;

        QFont mLineNumberFont;
        QColor mLineNumberColor;
        QColor mLineNumberBackground;
        QColor mLineNumberHighlightColor;
        QColor mLineNumberHighlightBackground;
        QColor mCurrentLineBackground;
    };
}

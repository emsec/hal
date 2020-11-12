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
        explicit CodeEditor(QWidget* parent = nullptr);

        virtual bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

        void lineNumberAreaPaintEvent(QPaintEvent* event);
        void minimapPaintEvent(QPaintEvent* event);

        int lineNumberAreaWidth();
        int minimapWidth();

        int first_visible_block();
        int visibleBlockCount();

        void centerOnLine(const int number);

        void handleWheelEvent(QWheelEvent* event);

        CodeEditorMinimap* minimap();

        QFont lineNumberFont() const;
        QColor lineNumberColor() const;
        QColor lineNumberBackground() const;
        QColor lineNumberHighlightColor() const;
        QColor lineNumberHighlightBackground() const;
        QColor currentLineBackground() const;

        void setLineNumberFont(QFont& font);
        void setLineNumberColor(QColor& color);
        void setLineNumberBackground(QColor& color);
        void setLineNumberHighlightColor(QColor& color);
        void setLineNumberHighlightBackground(QColor& color);
        void setCurrentLineBackground(QColor& color);

    public Q_SLOTS:
        void search(const QString& string);

        void toggleLineNumbers();
        void toggleMinimap();

    protected:
        virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    private Q_SLOTS:
        void highlightCurrentLine();
        void handleBlockCountChanged(int new_block_count);
        void updateLineNumberArea(const QRect& rect, int dy);
        void updateMinimap(const QRect& rect, int dy);
        void handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value);

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

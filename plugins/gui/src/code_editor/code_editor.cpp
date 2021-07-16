#include "gui/code_editor/code_editor.h"

#include "gui/code_editor/code_editor_scrollbar.h"
#include "gui/code_editor/line_number_area.h"
#include "gui/code_editor/minimap_scrollbar.h"

#include "gui/gui_globals.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QTextBlock>
#include <QDebug>

namespace hal
{

    CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent),
        mScrollbar(new CodeEditorScrollbar(this)),
        mLineNumberArea(new LineNumberArea(this)),
        mMinimap(new CodeEditorMinimap(this)),
        mAnimation(new QPropertyAnimation(mScrollbar, "value", this)),
        mLineNumbersEnabled(true),
        mLineHighlightEnabled(true),
        mMinimapEnabled(false),
        mLineWrapEnabled(false)
    {
        connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::handleBlockCountChanged);
        connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
        connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateMinimap);

        if (mLineHighlightEnabled)
        {
            connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
        }

        setVerticalScrollBar(mScrollbar);
        mScrollbar->setMinimapScrollbar(mMinimap->scrollbar());
        mMinimap->scrollbar()->setScrollbar(mScrollbar);

        mScrollbar->setContextMenuPolicy(Qt::NoContextMenu);
        horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

        mAnimation->setDuration(200);

        setLineWrapMode(mLineWrapEnabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        setFrameStyle(QFrame::NoFrame);

        ensurePolished();
        updateLayout();

        installEventFilter(this);

        document()->setDocumentMargin(0);
    }

    bool CodeEditor::eventFilter(QObject* object, QEvent* event)
    {

        Q_UNUSED(object)

        if (event->type() == QEvent::Wheel)
        {
            QWheelEvent* wheel = static_cast<QWheelEvent*>(event);
            if (wheel->modifiers() == Qt::ControlModifier)
            {
                if (wheel->delta() > 0)
                    zoomIn(2);
                else
                    zoomOut(2);

                return true;
            }
        }
        return false;
    }

    void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
    {
        QPainter painter(mLineNumberArea);
        painter.setFont(mLineNumberFont);
        QPen pen;
        pen.setColor(mLineNumberColor);
        painter.setPen(pen);

        QTextBlock block = firstVisibleBlock();
        int block_number = block.blockNumber();
        qreal top        = blockBoundingGeometry(block).translated(contentOffset()).top();
        int bottom       = event->rect().bottom();
        int width        = mLineNumberArea->width() - mLineNumberArea->rightOffset();

        while (block.isValid() && top <= bottom)
        {
            block_number++;    // INCREMENT HERE SO FIRST BLOCK HAS INDEX 1 INSTEAD OF 0

            if (block.isVisible())
            {
                top          = blockBoundingGeometry(block).translated(contentOffset()).top();
                qreal height = blockBoundingGeometry(block).height();
                painter.drawText(QRectF(0, top, width, height), Qt::AlignRight | Qt::AlignVCenter, QString::number(block_number));
            }

            block = block.next();
        }
    }

    void CodeEditor::minimapPaintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event)

        // MIGHT BE NEEDED LATER
    }

    int CodeEditor::lineNumberAreaWidth()
    {
        // WARNING FUNCTION ONLY RETURNS CORRECT VALUES FOR MONOSPACE FONTS !
        QFontMetrics fm(mLineNumberFont);
        return mLineNumberArea->leftOffset() + fm.width(QString::number(blockCount())) + mLineNumberArea->rightOffset();
    }

    int CodeEditor::minimapWidth()
    {
        return 160;
    }

    void CodeEditor::resizeEvent(QResizeEvent* event)
    {
        updateLayout();

        qreal ratio = viewport()->height() / blockBoundingGeometry(document()->firstBlock()).height();
        mMinimap->adjustSliderHeight(ratio);

        QPlainTextEdit::resizeEvent(event);
    }

    void CodeEditor::clearLineHighlight()
    {
        QList<QTextEdit::ExtraSelection> no_selections;
        setExtraSelections(no_selections);
    }

    void CodeEditor::highlightCurrentLine()
    {
        /*
         * Qt doesn't want to highlight blocks in line-wrap mode (even if the docs
         * say so), so we hack our way around it by going to the end of a block and
         * then go upwards selecting each line in that block separately.
         */
        QList<QTextEdit::ExtraSelection> extra_selections;
        QTextCursor cursor(textCursor());
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.movePosition(QTextCursor::StartOfLine);
        int block = cursor.blockNumber();
        int oldPosition;

        do
        {
            QTextEdit::ExtraSelection selection;

            selection.format.setBackground(mCurrentLineBackground);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = cursor;
            extra_selections.append(selection);

            oldPosition = cursor.position();
            cursor.movePosition(QTextCursor::Up);
        // stop when we leave the block or hit the top of the document
        } while(cursor.blockNumber() == block && cursor.position() != oldPosition);

        setExtraSelections(extra_selections);
    }

    void CodeEditor::handleBlockCountChanged(int new_block_count)
    {
        Q_UNUSED(new_block_count);

        updateLayout();

        qreal ratio = viewport()->height() / blockBoundingGeometry(document()->firstBlock()).height();
        mMinimap->adjustSliderHeight(ratio);
    }

    void CodeEditor::updateLineNumberArea(const QRect& rect, int dy)
    {
        Q_UNUSED(rect)
        Q_UNUSED(dy)

        mLineNumberArea->update();
    }

    void CodeEditor::updateMinimap(const QRect& rect, int dy)
    {
        Q_UNUSED(rect)
        Q_UNUSED(dy)

        mMinimap->update();
    }

    void CodeEditor::search(const QString& string, QTextDocument::FindFlags options)
    {
        QList<QTextEdit::ExtraSelection> extraSelections;

        moveCursor(QTextCursor::Start);
        QColor color            = QColor(12, 15, 19);
        QColor mBackgroundColor = QColor(255, 255, 0);

        while (find(string, options))
        {
            QTextEdit::ExtraSelection extra;
            extra.format.setForeground(QBrush(color));
            extra.format.setBackground(mBackgroundColor);
            extra.cursor = textCursor();
            extraSelections.append(extra);
        }
        setExtraSelections(extraSelections);
    }

    void CodeEditor::toggleLineNumbers()
    {
        mLineNumbersEnabled = !mLineNumbersEnabled;
        updateLayout();
    }

    void CodeEditor::toggleMinimap()
    {
        mMinimapEnabled = !mMinimapEnabled;
        updateLayout();
    }

    int CodeEditor::first_visible_block()
    {
        return firstVisibleBlock().blockNumber();
    }

    void CodeEditor::centerOnLine(const int number)
    {
        int total = document()->lineCount();

        if (number < 0 || number >= total)
            return;

        if (mAnimation->state() == QPropertyAnimation::Running)
            mAnimation->stop();

        mAnimation->setStartValue(verticalScrollBar()->value());
        mAnimation->setEndValue(number);
        mAnimation->start();
    }

    void CodeEditor::setFontSize(int pt)
    {
        QFont font = QPlainTextEdit::font();
        font.setPointSize(pt);
        QPlainTextEdit::setFont(font);
    }

    void CodeEditor::handleWheelEvent(QWheelEvent* event)
    {
        QPlainTextEdit::wheelEvent(event);
    }

    CodeEditorMinimap* CodeEditor::minimap()
    {
        return mMinimap;
    }

    QFont CodeEditor::lineNumberFont() const
    {
        return mLineNumberFont;
    }

    QColor CodeEditor::lineNumberColor() const
    {
        return mLineNumberColor;
    }

    QColor CodeEditor::lineNumberBackground() const
    {
        return mLineNumberBackground;
    }

    QColor CodeEditor::lineNumberHighlightColor() const
    {
        return mLineNumberHighlightColor;
    }

    QColor CodeEditor::lineNumberHighlightBackground() const
    {
        return mLineNumberHighlightBackground;
    }

    QColor CodeEditor::currentLineBackground() const
    {
        return mCurrentLineBackground;
    }

    void CodeEditor::setLineNumberFont(const QFont &font)
    {
        mLineNumberFont = font;
    }

    void CodeEditor::setLineNumberColor(QColor& color)
    {
        mLineNumberColor = color;
    }

    void CodeEditor::setLineNumberBackground(QColor& color)
    {
        mLineNumberBackground = color;
    }

    void CodeEditor::setLineNumberHighlightColor(QColor& color)
    {
        mLineNumberHighlightColor = color;
    }

    void CodeEditor::setLineNumberHighlightBackground(QColor& color)
    {
        mLineNumberHighlightBackground = color;
    }

    void CodeEditor::setCurrentLineBackground(QColor& color)
    {
        mCurrentLineBackground = color;
    }

    void CodeEditor::updateLayout()
    {
        int left_margin = 0;
        int right_margin = 0;

        QRect cr = contentsRect();

        if (mLineNumbersEnabled)
        {
            left_margin = lineNumberAreaWidth();
            mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), viewport()->height()));
            mLineNumberArea->show();
        }
        else
            mLineNumberArea->hide();

        if (mMinimapEnabled)
        {
            right_margin = minimapWidth();
            mMinimap->setGeometry(QRect(cr.right() - minimapWidth(), cr.top(), minimapWidth(), viewport()->height()));
            mMinimap->show();
        }
        else
            mMinimap->hide();

        setViewportMargins(left_margin, 0, right_margin, 0);
    }

    void CodeEditor::setLineNumberEnabled(bool enabled)
    {
        mLineNumbersEnabled = enabled;
        updateLayout();
    }

    void CodeEditor::setHighlightCurrentLineEnabled(bool enabled)
    {
        mLineHighlightEnabled = enabled;

            if(enabled)
            {
                connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
                highlightCurrentLine();
            }
            else
            {
                disconnect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
                clearLineHighlight();
            }

        updateLayout();
    }

    void CodeEditor::setLineWrapEnabled(bool enabled)
    {
        mLineWrapEnabled = enabled;
        setLineWrapMode(mLineWrapEnabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        updateLayout();
    }

    void CodeEditor::setMinimapEnabled(bool enabled)
    {
        mMinimapEnabled = enabled;
        updateLayout();
    }
}

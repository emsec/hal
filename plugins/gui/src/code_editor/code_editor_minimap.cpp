#include "gui/code_editor/code_editor_minimap.h"

#include "gui/code_editor/code_editor.h"
#include "gui/code_editor/minimap_scrollbar.h"

#include <QPainter>
#include <QStyleOption>
#include <QTextBlock>

#include <cmath>
#include <math.h>

#include <QDebug>
namespace hal
{
    CodeEditorMinimap::CodeEditorMinimap(CodeEditor* editor)
        : QWidget(editor), mEditor(editor), mDocument(new QTextDocument()), mScrollbar(new MinimapScrollbar(this)), mDocumentHeight(0), mOffset(0)
    {
        connect(mEditor->document(), &QTextDocument::contentsChange, this, &CodeEditorMinimap::handleContentsChange);
        connect(mDocument->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &CodeEditorMinimap::handleDocumentSizeChanged);

        mDocument->setDocumentMargin(0);
        mScrollbar->show();

        repolish();
    }

    MinimapScrollbar* CodeEditorMinimap::scrollbar()
    {
        return mScrollbar;
    }

    QTextDocument* CodeEditorMinimap::document()
    {
        return mDocument;
    }

    void CodeEditorMinimap::adjustSliderHeight(int viewport_height)
    {
        //mScrollbar->setSliderHeight(viewport_height * 0.25f);

        //    qreal doc_height = mEditor->document()->documentLayout()->documentSize().height();
        //    qreal ratio = doc_height / mEditor->viewport()->contentsRect().height();

        //    doc_height = mDocument->documentLayout()->documentSize().height();
        //    mScrollbar->setSliderHeight(std::round(doc_height / ratio));
        Q_UNUSED(viewport_height);
        qDebug() << "editor " << mEditor->document()->documentLayout()->documentSize().height();
        qDebug() << "mini " << mDocument->documentLayout()->documentSize().height();

        qreal ratio = mEditor->document()->documentLayout()->documentSize().height() / mDocument->documentLayout()->documentSize().height();    // UNEXPECTED RESULT, MAKES NO SENSE

        mScrollbar->setSliderHeight(std::round(mEditor->viewport()->contentsRect().height() * ratio));
    }

    void CodeEditorMinimap::adjustSliderHeight(qreal ratio)
    {
        mScrollbar->setSliderHeight(std::round(ratio * mDocument->documentLayout()->blockBoundingRect(mDocument->firstBlock()).height()));
        resizeScrollbar();
    }

    void CodeEditorMinimap::adjustSliderHeight(int first_visible_block, int last_visible_block)
    {
        qDebug() << "first block: " + QString::number(first_visible_block);
        qDebug() << "last block: " + QString::number(last_visible_block);

        qreal top    = mDocument->documentLayout()->blockBoundingRect(mDocument->findBlockByNumber(first_visible_block)).top();
        qreal bottom = mDocument->documentLayout()->blockBoundingRect(mDocument->findBlockByNumber(last_visible_block)).bottom();

        qDebug() << "top: " + QString::number(top);
        qDebug() << "bottom: " + QString::number(bottom);

        mScrollbar->setSliderHeight(std::round(bottom - top));
    }

    void CodeEditorMinimap::handleDocumentSizeChanged(const QSizeF& new_size)
    {
        mDocumentHeight = std::ceil(new_size.height());
        resizeScrollbar();
    }

    void CodeEditorMinimap::handleContentsChange(int position, int chars_removed, int chars_added)
    {
        QTextCursor cursor = QTextCursor(mDocument);
        cursor.setPosition(position);

        if (chars_removed)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, chars_removed);
            cursor.removeSelectedText();
        }

        if (chars_added)
            cursor.insertText(mEditor->document()->toPlainText().mid(position, chars_added));
    }

    void CodeEditorMinimap::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event)

        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

        painter.setClipping(true);      // UNCERTAIN IF NECESSARY
        painter.setClipRect(rect());    // UNCERTAIN IF NECESSARY

        //    int block_number = mEditor->first_visible_block();
        //    int sliderPosition = mScrollbar->sliderPosition();
        //    qreal offset = mDocument->documentLayout()->blockBoundingRect(mDocument->findBlockByNumber(block_number)).top() - sliderPosition;
        //    painter.translate(0, -offset);

        //    QAbstractTextDocumentLayout::PaintContext ctx;
        //    ctx.palette.setColor(QPalette::Text, QColor("yellow")); // USE STYLESHEET COLOR ?
        //    ctx.clip = QRectF(0, offset, width(), height());

        //    mDocument->documentLayout()->draw(&painter, ctx);

        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, palette().text().color());

        if (mDocumentHeight > height())
        {
            int block_number    = mEditor->first_visible_block();
            int sliderPosition = mScrollbar->sliderPosition();
            mOffset            = mDocument->documentLayout()->blockBoundingRect(mDocument->findBlockByNumber(block_number)).top() - sliderPosition;
            painter.translate(0, -mOffset);

            ctx.clip = QRectF(0, mOffset, width(), height());
        }

        mDocument->documentLayout()->draw(&painter, ctx);
    }

    void CodeEditorMinimap::resizeEvent(QResizeEvent* event)
    {
        Q_UNUSED(event)

        resizeScrollbar();
    }

    void CodeEditorMinimap::mousePressEvent(QMouseEvent* event)
    {
        int position = mDocument->documentLayout()->hitTest(QPointF(event->pos().x(), event->pos().y() + mOffset), Qt::FuzzyHit);
        QTextCursor cursor(mDocument);
        cursor.setPosition(position);
        mEditor->centerOnLine(cursor.blockNumber());
    }

    void CodeEditorMinimap::wheelEvent(QWheelEvent* event)
    {
        mEditor->handleWheelEvent(event);
    }

    void CodeEditorMinimap::resizeScrollbar()
    {
        if (mDocumentHeight < height())
            mScrollbar->resize(width(), std::max(mScrollbar->sliderHeight(), mDocumentHeight));
        else
            mScrollbar->resize(width(), height());
    }

    void CodeEditorMinimap::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mDocument->setDefaultFont(font());
    }
}

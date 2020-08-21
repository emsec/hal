#include "code_editor/code_editor_minimap.h"

#include "code_editor/code_editor.h"
#include "code_editor/minimap_scrollbar.h"

#include <QPainter>
#include <QStyleOption>
#include <QTextBlock>

#include <cmath>
#include <math.h>

#include <QDebug>
namespace hal
{
    CodeEditorMinimap::CodeEditorMinimap(CodeEditor* editor)
        : QWidget(editor), m_editor(editor), m_document(new QTextDocument()), m_scrollbar(new MinimapScrollbar(this)), m_document_height(0), m_offset(0)
    {
        connect(m_editor->document(), &QTextDocument::contentsChange, this, &CodeEditorMinimap::handle_contents_change);
        connect(m_document->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &CodeEditorMinimap::handle_document_size_changed);

        m_document->setDocumentMargin(0);
        m_scrollbar->show();

        repolish();
    }

    MinimapScrollbar* CodeEditorMinimap::scrollbar()
    {
        return m_scrollbar;
    }

    QTextDocument* CodeEditorMinimap::document()
    {
        return m_document;
    }

    void CodeEditorMinimap::adjust_slider_height(int viewport_height)
    {
        //m_scrollbar->set_slider_height(viewport_height * 0.25f);

        //    qreal doc_height = m_editor->document()->documentLayout()->documentSize().height();
        //    qreal ratio = doc_height / m_editor->viewport()->contentsRect().height();

        //    doc_height = m_document->documentLayout()->documentSize().height();
        //    m_scrollbar->set_slider_height(std::round(doc_height / ratio));
        Q_UNUSED(viewport_height);
        qDebug() << "editor " << m_editor->document()->documentLayout()->documentSize().height();
        qDebug() << "mini " << m_document->documentLayout()->documentSize().height();

        qreal ratio = m_editor->document()->documentLayout()->documentSize().height() / m_document->documentLayout()->documentSize().height();    // UNEXPECTED RESULT, MAKES NO SENSE

        m_scrollbar->set_slider_height(std::round(m_editor->viewport()->contentsRect().height() * ratio));
    }

    void CodeEditorMinimap::adjust_slider_height(qreal ratio)
    {
        m_scrollbar->set_slider_height(std::round(ratio * m_document->documentLayout()->blockBoundingRect(m_document->firstBlock()).height()));
        resize_scrollbar();
    }

    void CodeEditorMinimap::adjust_slider_height(int first_visible_block, int last_visible_block)
    {
        qDebug() << "first block: " + QString::number(first_visible_block);
        qDebug() << "last block: " + QString::number(last_visible_block);

        qreal top    = m_document->documentLayout()->blockBoundingRect(m_document->findBlockByNumber(first_visible_block)).top();
        qreal bottom = m_document->documentLayout()->blockBoundingRect(m_document->findBlockByNumber(last_visible_block)).bottom();

        qDebug() << "top: " + QString::number(top);
        qDebug() << "bottom: " + QString::number(bottom);

        m_scrollbar->set_slider_height(std::round(bottom - top));
    }

    void CodeEditorMinimap::handle_document_size_changed(const QSizeF& new_size)
    {
        m_document_height = std::ceil(new_size.height());
        resize_scrollbar();
    }

    void CodeEditorMinimap::handle_contents_change(int position, int chars_removed, int chars_added)
    {
        QTextCursor cursor = QTextCursor(m_document);
        cursor.setPosition(position);

        if (chars_removed)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, chars_removed);
            cursor.removeSelectedText();
        }

        if (chars_added)
            cursor.insertText(m_editor->document()->toPlainText().mid(position, chars_added));
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

        //    int block_number = m_editor->first_visible_block();
        //    int slider_position = m_scrollbar->slider_position();
        //    qreal offset = m_document->documentLayout()->blockBoundingRect(m_document->findBlockByNumber(block_number)).top() - slider_position;
        //    painter.translate(0, -offset);

        //    QAbstractTextDocumentLayout::PaintContext ctx;
        //    ctx.palette.setColor(QPalette::Text, QColor("yellow")); // USE STYLESHEET COLOR ?
        //    ctx.clip = QRectF(0, offset, width(), height());

        //    m_document->documentLayout()->draw(&painter, ctx);

        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, palette().text().color());

        if (m_document_height > height())
        {
            int block_number    = m_editor->first_visible_block();
            int slider_position = m_scrollbar->slider_position();
            m_offset            = m_document->documentLayout()->blockBoundingRect(m_document->findBlockByNumber(block_number)).top() - slider_position;
            painter.translate(0, -m_offset);

            ctx.clip = QRectF(0, m_offset, width(), height());
        }

        m_document->documentLayout()->draw(&painter, ctx);
    }

    void CodeEditorMinimap::resizeEvent(QResizeEvent* event)
    {
        Q_UNUSED(event)

        resize_scrollbar();
    }

    void CodeEditorMinimap::mousePressEvent(QMouseEvent* event)
    {
        int position = m_document->documentLayout()->hitTest(QPointF(event->pos().x(), event->pos().y() + m_offset), Qt::FuzzyHit);
        QTextCursor cursor(m_document);
        cursor.setPosition(position);
        m_editor->center_on_line(cursor.blockNumber());
    }

    void CodeEditorMinimap::wheelEvent(QWheelEvent* event)
    {
        m_editor->handle_wheel_event(event);
    }

    void CodeEditorMinimap::resize_scrollbar()
    {
        if (m_document_height < height())
            m_scrollbar->resize(width(), std::max(m_scrollbar->slider_height(), m_document_height));
        else
            m_scrollbar->resize(width(), height());
    }

    void CodeEditorMinimap::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        m_document->setDefaultFont(font());
    }
}

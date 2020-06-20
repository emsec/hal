#include "code_editor/code_editor.h"

#include "code_editor/code_editor_scrollbar.h"
#include "code_editor/line_number_area.h"
#include "code_editor/minimap_scrollbar.h"

#include "gui/gui_globals.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QTextBlock>
namespace hal
{

    CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent),
          m_scrollbar(new CodeEditorScrollbar(this)),
          m_line_number_area(new LineNumberArea(this)),
          m_minimap(new CodeEditorMinimap(this)),
          m_animation(new QPropertyAnimation(m_scrollbar, "value", this))
    {
        connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::handle_block_count_changed);
        connect(this, &CodeEditor::updateRequest, this, &CodeEditor::update_line_number_area);
        connect(this, &CodeEditor::updateRequest, this, &CodeEditor::update_minimap);

        m_line_highlight_enabled = g_settings_manager.get("python/highlight_current_line").toBool();
        if (m_line_highlight_enabled)
        {
            connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlight_current_line);
        }
        m_line_numbers_enabled = g_settings_manager.get("python/line_numbers").toBool();
        m_line_wrap_enabled = g_settings_manager.get("python/line_wrap").toBool();
        m_minimap_enabled = g_settings_manager.get("python/minimap").toBool();

        connect(&g_settings_relay, &settings_relay::setting_changed, this, &CodeEditor::handle_global_setting_changed);

        setVerticalScrollBar(m_scrollbar);
        m_scrollbar->set_minimap_scrollbar(m_minimap->scrollbar());
        m_minimap->scrollbar()->set_scrollbar(m_scrollbar);

        m_scrollbar->setContextMenuPolicy(Qt::NoContextMenu);
        horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

        m_animation->setDuration(200);

        setLineWrapMode(m_line_wrap_enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        setFrameStyle(QFrame::NoFrame);

        ensurePolished();
        update_layout();

        //installEventFilter(this);

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
                    zoomIn(1);
                else
                    zoomOut(1);

                return true;
            }
        }
        return false;
    }

    void CodeEditor::line_number_area_paint_event(QPaintEvent* event)
    {
        QPainter painter(m_line_number_area);
        painter.setFont(m_line_number_font);
        QPen pen;
        pen.setColor(m_line_number_color);
        painter.setPen(pen);

        QTextBlock block = firstVisibleBlock();
        int block_number = block.blockNumber();
        qreal top        = blockBoundingGeometry(block).translated(contentOffset()).top();
        int bottom       = event->rect().bottom();
        int width        = m_line_number_area->width() - m_line_number_area->right_offset();

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

    void CodeEditor::minimap_paint_event(QPaintEvent* event)
    {
        Q_UNUSED(event)

        // MIGHT BE NEEDED LATER
    }

    int CodeEditor::line_number_area_width()
    {
        // ADD MID SPACE FOR COLOR INDICATORS ?
        // WARNING FUNCTION ONLY RETURNS CORRECT VALUES FOR MONOSPACE FONTS !
        QFontMetrics fm(m_line_number_font);
        return m_line_number_area->left_offset() + fm.width(QString::number(blockCount())) + m_line_number_area->right_offset();
    }

    int CodeEditor::minimap_width()
    {
        // SET VIA STYLESHEET OR DYNAMICALLY ?
        return 160;
    }

    void CodeEditor::resizeEvent(QResizeEvent* event)
    {
        update_layout();

        //    QTextBlock block = firstVisibleBlock();
        //    int first_visible_block = block.blockNumber();
        //    int additional_blocks = 0;

        //    int block_top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
        //    int viewport_height = viewport()->height();

        //    block = block.next();
        //    while (block.isValid() && block_top <= viewport_height)
        //    {
        //        block = block.next();
        //        additional_blocks++;

        //        if (block.isVisible())
        //            block_top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
        //    }

        //    // ADJUST TO PERCENTAGE OF LAST BLOCK ?
        //    m_minimap->adjust_slider_height(first_visible_block, first_visible_block + additional_blocks);

        qreal ratio = viewport()->height() / blockBoundingGeometry(document()->firstBlock()).height();
        m_minimap->adjust_slider_height(ratio);

        QPlainTextEdit::resizeEvent(event);
    }

    void CodeEditor::clear_line_highlight()
    {
        QList<QTextEdit::ExtraSelection> no_selections;
        setExtraSelections(no_selections);
    }

    void CodeEditor::highlight_current_line()
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

            selection.format.setBackground(m_current_line_background);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = cursor;
            extra_selections.append(selection);

            oldPosition = cursor.position();
            cursor.movePosition(QTextCursor::Up);
        // stop when we leave the block or hit the top of the document
        } while(cursor.blockNumber() == block && cursor.position() != oldPosition);

        setExtraSelections(extra_selections);
    }

    void CodeEditor::handle_block_count_changed(int new_block_count)
    {
        Q_UNUSED(new_block_count);

        update_layout();

        qreal ratio = viewport()->height() / blockBoundingGeometry(document()->firstBlock()).height();
        m_minimap->adjust_slider_height(ratio);
    }

    void CodeEditor::update_line_number_area(const QRect& rect, int dy)
    {
        Q_UNUSED(rect)
        Q_UNUSED(dy)

        m_line_number_area->update();
    }

    void CodeEditor::update_minimap(const QRect& rect, int dy)
    {
        Q_UNUSED(rect)
        Q_UNUSED(dy)

        m_minimap->update();
    }

    void CodeEditor::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender);
        if (key == "python/highlight_current_line")
        {
            bool enable = value.toBool();
            if (enable == m_line_highlight_enabled)
                return;
            m_line_highlight_enabled = enable;
            if (enable)
            {
                connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlight_current_line);
                highlight_current_line();
            }
            else
            {
                disconnect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlight_current_line);
                clear_line_highlight();
            }
        }
        else if (key == "python/line_numbers")
        {
            m_line_numbers_enabled = value.toBool();
            update_layout();
        }
        else if (key == "python/line_wrap")
        {
            m_line_wrap_enabled = value.toBool();
            setLineWrapMode(m_line_wrap_enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        }
        else if (key == "python/minimap")
        {
            m_minimap_enabled = value.toBool();
            update_layout();
        }
    }

    void CodeEditor::search(const QString& string)
    {
        // THREAD ?
        QList<QTextEdit::ExtraSelection> extraSelections;

        moveCursor(QTextCursor::Start);
        // SET COLORS VIA STYLESHEET ?
        QColor color            = QColor(12, 15, 19);
        QColor background_color = QColor(255, 255, 0);

        while (find(string))
        {
            QTextEdit::ExtraSelection extra;
            extra.format.setForeground(QBrush(color));
            extra.format.setBackground(background_color);
            extra.cursor = textCursor();
            extraSelections.append(extra);
        }
        setExtraSelections(extraSelections);
    }

    void CodeEditor::toggle_line_numbers()
    {
        m_line_numbers_enabled = !m_line_numbers_enabled;
        update_layout();
    }

    void CodeEditor::toggle_minimap()
    {
        m_minimap_enabled = !m_minimap_enabled;
        update_layout();
    }

    int CodeEditor::first_visible_block()
    {
        return firstVisibleBlock().blockNumber();
    }

    int CodeEditor::visible_block_count()
    {
        // MIGHT BE NEEDED LATER, ADDED RETURN VALUE TO AVOID WARNING
        return 0;
    }

    void CodeEditor::center_on_line(const int number)
    {
        int total = document()->lineCount();

        if (number < 0 || number >= total)
            return;

        //    int hidden = verticalScrollBar()->maximum() - verticalScrollBar()->minimum();
        //    int visible = total - hidden;

        //    int value = number - (visible / 2);

        //    if (value < 0)
        //        value = 0;

        //    if (value >= hidden)
        //        value = hidden;

        //verticalScrollBar()->setValue(number); // CENTER ?

        if (m_animation->state() == QPropertyAnimation::Running)
            m_animation->stop();

        m_animation->setStartValue(verticalScrollBar()->value());
        m_animation->setEndValue(number);
        m_animation->start();
    }

    void CodeEditor::handle_wheel_event(QWheelEvent* event)
    {
        QPlainTextEdit::wheelEvent(event);
    }

    CodeEditorMinimap* CodeEditor::minimap()
    {
        return m_minimap;
    }

    QFont CodeEditor::line_number_font() const
    {
        return m_line_number_font;
    }

    QColor CodeEditor::line_number_color() const
    {
        return m_line_number_color;
    }

    QColor CodeEditor::line_number_background() const
    {
        return m_line_number_background;
    }

    QColor CodeEditor::line_number_highlight_color() const
    {
        return m_line_number_highlight_color;
    }

    QColor CodeEditor::line_number_highlight_background() const
    {
        return m_line_number_highlight_background;
    }

    QColor CodeEditor::current_line_background() const
    {
        return m_current_line_background;
    }

    void CodeEditor::set_line_number_font(QFont& font)
    {
        m_line_number_font = font;
    }

    void CodeEditor::set_line_number_color(QColor& color)
    {
        m_line_number_color = color;
    }

    void CodeEditor::set_line_number_background(QColor& color)
    {
        m_line_number_background = color;
    }

    void CodeEditor::set_line_number_highlight_color(QColor& color)
    {
        m_line_number_highlight_color = color;
    }

    void CodeEditor::set_line_number_highlight_background(QColor& color)
    {
        m_line_number_highlight_background = color;
    }

    void CodeEditor::set_current_line_background(QColor& color)
    {
        m_current_line_background = color;
    }

    void CodeEditor::update_layout()
    {
        int left_margin = 0;
        int right_margin = 0;

        QRect cr = contentsRect();

        if (m_line_numbers_enabled)
        {
            left_margin = line_number_area_width();
            m_line_number_area->setGeometry(QRect(cr.left(), cr.top(), line_number_area_width(), viewport()->height()));
            m_line_number_area->show();
        }
        else
            m_line_number_area->hide();

        if (m_minimap_enabled)
        {
            right_margin = minimap_width();
            m_minimap->setGeometry(QRect(cr.right() - minimap_width(), cr.top(), minimap_width(), viewport()->height()));
            m_minimap->show();
        }
        else
            m_minimap->hide();

        setViewportMargins(left_margin, 0, right_margin, 0);
    }
}

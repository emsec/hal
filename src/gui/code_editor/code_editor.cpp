#include "code_editor/code_editor.h"

#include "code_editor/code_editor_scrollbar.h"
#include "code_editor/line_number_area.h"
#include "code_editor/minimap_scrollbar.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QTextBlock>

code_editor::code_editor(QWidget* parent) : QPlainTextEdit(parent),
      m_scrollbar(new code_editor_scrollbar(this)),
      m_line_number_area(new line_number_area(this)),
      m_minimap(new code_editor_minimap(this)),
      m_animation(new QPropertyAnimation(m_scrollbar, "value", this)),
      m_line_numbers_enabled(true),
      m_minimap_enabled(true)
{
    connect(this, &code_editor::blockCountChanged, this, &code_editor::handle_block_count_changed);
    connect(this, &code_editor::updateRequest, this, &code_editor::update_line_number_area);
    connect(this, &code_editor::updateRequest, this, &code_editor::update_minimap);

    // CONNECT / DISCONNECT VIA SETTING ?
    //connect(this, &code_editor::cursorPositionChanged, this, &code_editor::highlight_current_line);

    setVerticalScrollBar(m_scrollbar);
    m_scrollbar->set_minimap_scrollbar(m_minimap->scrollbar());
    m_minimap->scrollbar()->set_scrollbar(m_scrollbar);

    m_scrollbar->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_animation->setDuration(200);

    // SET LINEWRAP MODE VIA SETTING ?
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setFrameStyle(QFrame::NoFrame);

    ensurePolished();
    update_layout();

    //installEventFilter(this);

    document()->setDocumentMargin(0);
}

bool code_editor::eventFilter(QObject* object, QEvent* event)
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

void code_editor::line_number_area_paint_event(QPaintEvent* event)
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

void code_editor::minimap_paint_event(QPaintEvent* event)
{
    Q_UNUSED(event)

    // MIGHT BE NEEDED LATER
}

int code_editor::line_number_area_width()
{
    // ADD MID SPACE FOR COLOR INDICATORS ?
    // WARNING FUNCTION ONLY RETURNS CORRECT VALUES FOR MONOSPACE FONTS !
    QFontMetrics fm(m_line_number_font);
    return m_line_number_area->left_offset() + fm.width(QString::number(blockCount())) + m_line_number_area->right_offset();
}

int code_editor::minimap_width()
{
    // SET VIA STYLESHEET OR DYNAMICALLY ?
    return 160;
}

void code_editor::resizeEvent(QResizeEvent* event)
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

void code_editor::highlight_current_line()
{
    QList<QTextEdit::ExtraSelection> extra_selections;
    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(m_current_line_background);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extra_selections.append(selection);
    setExtraSelections(extra_selections);
}

void code_editor::handle_block_count_changed(int new_block_count)
{
    Q_UNUSED(new_block_count);

    update_layout();

    qreal ratio = viewport()->height() / blockBoundingGeometry(document()->firstBlock()).height();
    m_minimap->adjust_slider_height(ratio);
}

void code_editor::update_line_number_area(const QRect& rect, int dy)
{
    Q_UNUSED(rect)
    Q_UNUSED(dy)

    m_line_number_area->update();
}

void code_editor::update_minimap(const QRect& rect, int dy)
{
    Q_UNUSED(rect)
    Q_UNUSED(dy)

    m_minimap->update();
}

void code_editor::search(const QString& string)
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

void code_editor::toggle_line_numbers()
{
    m_line_numbers_enabled = !m_line_numbers_enabled;
    update_layout();
}

void code_editor::toggle_minimap()
{
    m_minimap_enabled = !m_minimap_enabled;
    update_layout();
}

int code_editor::first_visible_block()
{
    return firstVisibleBlock().blockNumber();
}

int code_editor::visible_block_count()
{
    // MIGHT BE NEEDED LATER, ADDED RETURN VALUE TO AVOID WARNING
    return 0;
}

void code_editor::center_on_line(const int number)
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

void code_editor::handle_wheel_event(QWheelEvent* event)
{
    QPlainTextEdit::wheelEvent(event);
}

code_editor_minimap* code_editor::minimap()
{
    return m_minimap;
}

QFont code_editor::line_number_font() const
{
    return m_line_number_font;
}

QColor code_editor::line_number_color() const
{
    return m_line_number_color;
}

QColor code_editor::line_number_background() const
{
    return m_line_number_background;
}

QColor code_editor::line_number_highlight_color() const
{
    return m_line_number_highlight_color;
}

QColor code_editor::line_number_highlight_background() const
{
    return m_line_number_highlight_background;
}

QColor code_editor::current_line_background() const
{
    return m_current_line_background;
}

void code_editor::set_line_number_font(QFont& font)
{
    m_line_number_font = font;
}

void code_editor::set_line_number_color(QColor& color)
{
    m_line_number_color = color;
}

void code_editor::set_line_number_background(QColor& color)
{
    m_line_number_background = color;
}

void code_editor::set_line_number_highlight_color(QColor& color)
{
    m_line_number_highlight_color = color;
}

void code_editor::set_line_number_highlight_background(QColor& color)
{
    m_line_number_highlight_background = color;
}

void code_editor::set_current_line_background(QColor& color)
{
    m_current_line_background = color;
}

void code_editor::update_layout()
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

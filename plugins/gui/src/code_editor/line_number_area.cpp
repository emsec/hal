#include "gui/code_editor/line_number_area.h"

#include "gui/code_editor/code_editor.h"

#include <QPainter>
#include <QStyleOption>
namespace hal
{
    LineNumberArea::LineNumberArea(CodeEditor* editor) : QWidget(editor), m_editor(editor)
    {
        repolish();
    }

    int LineNumberArea::left_offset() const
    {
        return m_left_offset;
    }

    int LineNumberArea::right_offset() const
    {
        return m_right_offset;
    }

    void LineNumberArea::set_left_offset(const int offset)
    {
        m_left_offset = offset;
    }

    void LineNumberArea::set_right_offset(const int offset)
    {
        m_right_offset = offset;
    }

    void LineNumberArea::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    void LineNumberArea::paintEvent(QPaintEvent* event)
    {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        m_editor->line_number_area_paint_event(event);
    }

    void LineNumberArea::wheelEvent(QWheelEvent* event)
    {
        m_editor->handle_wheel_event(event);
    }
}

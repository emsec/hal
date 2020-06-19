#include "code_editor/line_number_area.h"

#include "code_editor/code_editor.h"

#include <QPainter>
#include <QStyleOption>
namespace hal
{
    line_number_area::line_number_area(code_editor* editor) : QWidget(editor), m_editor(editor)
    {
        repolish();
    }

    int line_number_area::left_offset() const
    {
        return m_left_offset;
    }

    int line_number_area::right_offset() const
    {
        return m_right_offset;
    }

    void line_number_area::set_left_offset(const int offset)
    {
        m_left_offset = offset;
    }

    void line_number_area::set_right_offset(const int offset)
    {
        m_right_offset = offset;
    }

    void line_number_area::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    void line_number_area::paintEvent(QPaintEvent* event)
    {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        m_editor->line_number_area_paint_event(event);
    }

    void line_number_area::wheelEvent(QWheelEvent* event)
    {
        m_editor->handle_wheel_event(event);
    }
}

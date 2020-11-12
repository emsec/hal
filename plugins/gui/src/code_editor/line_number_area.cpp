#include "gui/code_editor/line_number_area.h"

#include "gui/code_editor/code_editor.h"

#include <QPainter>
#include <QStyleOption>
namespace hal
{
    LineNumberArea::LineNumberArea(CodeEditor* editor) : QWidget(editor), mEditor(editor)
    {
        repolish();
    }

    int LineNumberArea::leftOffset() const
    {
        return mLeftOffset;
    }

    int LineNumberArea::rightOffset() const
    {
        return mRightOffset;
    }

    void LineNumberArea::setLeftOffset(const int offset)
    {
        mLeftOffset = offset;
    }

    void LineNumberArea::setRightOffset(const int offset)
    {
        mRightOffset = offset;
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
        mEditor->lineNumberAreaPaintEvent(event);
    }

    void LineNumberArea::wheelEvent(QWheelEvent* event)
    {
        mEditor->handleWheelEvent(event);
    }
}

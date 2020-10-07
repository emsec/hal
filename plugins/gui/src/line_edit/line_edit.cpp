#include "gui/line_edit/line_edit.h"

#include <QStyle>

namespace hal
{
    LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent)
    {
        connect(this, &LineEdit::textChanged, this, &LineEdit::handle_text_changed);
    }

    LineEdit::LineEdit(const QString& contents, QWidget* parent) : QLineEdit(contents, parent)
    {
        connect(this, &LineEdit::textChanged, this, &LineEdit::handle_text_changed);
    }

    void LineEdit::handle_text_changed(const QString& text)
    {
        Q_UNUSED(text)

        style()->unpolish(this);
        style()->polish(this);
    }
}

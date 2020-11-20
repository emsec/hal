#include "gui/line_edit/line_edit.h"

#include <QStyle>

namespace hal
{
    LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent)
    {
        connect(this, &LineEdit::textChanged, this, &LineEdit::handleTextChanged);
    }

    LineEdit::LineEdit(const QString& contents, QWidget* parent) : QLineEdit(contents, parent)
    {
        connect(this, &LineEdit::textChanged, this, &LineEdit::handleTextChanged);
    }

    void LineEdit::handleTextChanged(const QString& text)
    {
        Q_UNUSED(text)

        style()->unpolish(this);
        style()->polish(this);
    }
}

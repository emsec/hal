#include "line_edit/line_edit.h"

#include <QStyle>
namespace hal{
line_edit::line_edit(QWidget* parent) : QLineEdit(parent)
{
    connect(this, &line_edit::textChanged, this, &line_edit::handle_text_changed);
}

line_edit::line_edit(const QString& contents, QWidget* parent) : QLineEdit(contents, parent)
{
    connect(this, &line_edit::textChanged, this, &line_edit::handle_text_changed);
}

void line_edit::handle_text_changed(const QString& text)
{
    Q_UNUSED(text)

    style()->unpolish(this);
    style()->polish(this);
}
}

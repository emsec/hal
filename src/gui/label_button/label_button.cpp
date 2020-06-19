#include "label_button/label_button.h"

namespace hal
{
    label_button::label_button(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f)
    {
    }

    label_button::label_button(const QString& text, QWidget* parent, Qt::WindowFlags f) : QLabel(text, parent, f)
    {
    }

    void label_button::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        Q_EMIT clicked();
    }
}

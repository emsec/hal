#include "label_button/label_button.h"

namespace hal
{
    LabelButton::LabelButton(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f)
    {
    }

    LabelButton::LabelButton(const QString& text, QWidget* parent, Qt::WindowFlags f) : QLabel(text, parent, f)
    {
    }

    void LabelButton::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        Q_EMIT clicked();
    }
}

#include "toggleable_label/toggleable_label.h"

toggleable_label::toggleable_label(QWidget *parent) : QLabel(parent)
{
}

toggleable_label::toggleable_label(const QString &text, QWidget *parent) : QLabel(text, parent)
{
}

void toggleable_label::toggle_visible()
{
    setVisible(isHidden());
}

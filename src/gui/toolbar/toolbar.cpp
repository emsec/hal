#include "toolbar/toolbar.h"

toolbar::toolbar(QWidget* parent) : QToolBar(parent)
{
}

void toolbar::add_spacer()
{
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(spacer);
}

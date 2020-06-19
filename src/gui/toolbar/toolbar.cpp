#include "toolbar/toolbar.h"

#include <QStyle>

namespace hal
{
    toolbar::toolbar(QWidget* parent) : QToolBar(parent)
    {
    }

    void toolbar::add_spacer()
    {
        QWidget* spacer = new QWidget(this);
        spacer->setAttribute(Qt::WA_NoSystemBackground);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        addWidget(spacer);
    }

    void toolbar::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }
}

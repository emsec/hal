#include "gui/toolbar/toolbar.h"

#include <QStyle>

namespace hal
{
    Toolbar::Toolbar(QWidget* parent) : QToolBar(parent)
    {
    }

    void Toolbar::addSpacer()
    {
        QWidget* spacer = new QWidget(this);
        spacer->setAttribute(Qt::WA_NoSystemBackground);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        addWidget(spacer);
    }

    void Toolbar::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }
}

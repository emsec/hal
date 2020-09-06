#include "gui/window_manager/workspace.h"

#include <QStyle>

namespace hal
{
    Workspace::Workspace(QWidget* parent) : QFrame(parent)
    {

    }

    void Workspace::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }
}

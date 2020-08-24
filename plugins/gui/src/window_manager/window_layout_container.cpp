#include "gui/window_manager/window_layout_container.h"

namespace hal
{
    WindowLayoutContainer::WindowLayoutContainer(QWidget* parent) : QFrame(parent)
    {

    }

    void WindowLayoutContainer::keyPressEvent(QKeyEvent* event)
    {
        // FORWARD (RELEVANT BUTTONS / EVENT) TO MANAGER
        QFrame::keyPressEvent(event);
    }
}

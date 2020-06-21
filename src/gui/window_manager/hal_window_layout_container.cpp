#include "window_manager/hal_window_layout_container.h"

namespace hal
{
    HalWindowLayoutContainer::HalWindowLayoutContainer(QWidget* parent) : QFrame(parent)
    {

    }

    void HalWindowLayoutContainer::keyPressEvent(QKeyEvent* event)
    {
        // FORWARD (RELEVANT BUTTONS / EVENT) TO MANAGER
        QFrame::keyPressEvent(event);
    }
}

#include "window_manager/hal_window_layout_container.h"

hal_window_layout_container::hal_window_layout_container(QWidget* parent) : QFrame(parent)
{

}

void hal_window_layout_container::keyPressEvent(QKeyEvent* event)
{
    // FORWARD (RELEVANT BUTTONS / EVENT) TO MANAGER
    QFrame::keyPressEvent(event);
}

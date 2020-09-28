#include "window_manager/window_effect_layer.h"

namespace hal
{
    WindowEffectLayer::WindowEffectLayer(QWidget* parent) : QFrame(parent)
    {

    }

    void WindowEffectLayer::keyPressEvent(QKeyEvent* event)
    {
        // FORWARD (RELEVANT BUTTONS / EVENT) TO MANAGER
        QFrame::keyPressEvent(event);
    }
}

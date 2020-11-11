#include "gui/gui_utils/geometry.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>

namespace hal
{
    namespace gui_utility
    {
        void ensureOnScreen(QWidget* w)
        {
            auto boundingRect = QApplication::desktop()->availableGeometry(w);
            auto requiredRect = w->geometry();
            // Try fitting vertically, prioritizing top on screen
            if (boundingRect.top() > requiredRect.top())
                requiredRect.moveTop(boundingRect.top());
            else if (boundingRect.bottom() < requiredRect.bottom())
                requiredRect.moveBottom(boundingRect.bottom());
            // Try fitting horizontally, prioritizing left on screen
            if (boundingRect.left() > requiredRect.left())
                requiredRect.moveLeft(boundingRect.left());
            else if (boundingRect.right() < requiredRect.right())
                requiredRect.moveRight(boundingRect.right());
            w->move(requiredRect.topLeft());
        }

    }
}

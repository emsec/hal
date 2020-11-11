//
// Created by Sebastian Wallat on 2019-04-16.
//

#include "gui/focus_logger/focus_logger.h"
#include "hal_core/utilities/log.h"
#include <QWidget>

namespace hal
{
    FocusLogger::FocusLogger(QApplication* app, QObject* parent) : QObject(parent)
    {
        connect(app, &QApplication::focusChanged, this, &FocusLogger::handleFocusChange);
    }

    void FocusLogger::handleFocusChange(QWidget* old, QWidget* new_obj)
    {
        if (old == nullptr && new_obj != nullptr)
        {
            log_info("gui", "GUI gained focus.");
        }
        else if (old != nullptr && new_obj == nullptr)
        {
            log_info("gui", "GUI lost focus.");
        }
    }
}

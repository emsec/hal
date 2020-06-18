//
// Created by Sebastian Wallat on 2019-04-16.
//

#include "gui/focus_logger/focus_logger.h"
#include "core/log.h"
#include <QWidget>
namespace hal{
focus_logger::focus_logger(QApplication* app, QObject* parent) : QObject(parent)
{
    connect(app, &QApplication::focusChanged, this, &focus_logger::handle_focus_change);
}

void focus_logger::handle_focus_change(QWidget* old, QWidget* new_obj)
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

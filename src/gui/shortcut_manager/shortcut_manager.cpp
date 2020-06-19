#include "shortcut_manager/shortcut_manager.h"

namespace hal
{
    shortcut_manager::shortcut_manager(QObject* parent) : QObject(parent)
    {
        // DOESNT WORK AS EXPECTED, USE FACTORY INSTEAD ?
        // ALTERNATIVELY MAKE THIS A HIDDEN TOP LEVEL WIDGET (HACK)
    }
}

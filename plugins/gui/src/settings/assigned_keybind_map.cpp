#include "gui/settings/assigned_keybind_map.h"
#include "gui/settings/settings_items/settings_item_keybind.h"

namespace hal
{
    AssignedKeybindMap* AssignedKeybindMap::inst = nullptr;

    AssignedKeybindMap* AssignedKeybindMap::instance()
    {
        if (!inst) inst = new AssignedKeybindMap;
        return inst;
    }

    SettingsItemKeybind* AssignedKeybindMap::currentAssignment(const QKeySequence& needle) const
    {
        return mKeybindMap.value(needle);
    }

    void AssignedKeybindMap::assign(const QKeySequence& newkey, SettingsItemKeybind *setting, const QKeySequence& oldkey)
    {
        if (!oldkey.isEmpty())
            mKeybindMap.remove(oldkey);
        mKeybindMap.insert(newkey,setting);
    }
}

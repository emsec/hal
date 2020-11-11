#include "gui/docking_system/dock_mime_data.h"

namespace hal
{
    DockMimeData::DockMimeData(ContentWidget* widget)
    {
        mWidget = widget;
    }

    ContentWidget* DockMimeData::widget() const
    {
        return mWidget;
    }
}

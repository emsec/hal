#include "gui/content_widget/dock_mime_data.h"

namespace hal
{
    DockMimeData::DockMimeData(ContentWidget* widget)
    {
        m_widget = widget;
    }

    ContentWidget* DockMimeData::widget() const
    {
        return m_widget;
    }
}

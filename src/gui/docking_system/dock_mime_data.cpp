#include "docking_system/dock_mime_data.h"

namespace hal
{
    dock_mime_data::dock_mime_data(content_widget* widget)
    {
        m_widget = widget;
    }

    content_widget* dock_mime_data::widget() const
    {
        return m_widget;
    }
}

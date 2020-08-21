#include "code_editor/code_editor_scrollbar.h"

#include "code_editor/minimap_scrollbar.h"
namespace hal
{
    CodeEditorScrollbar::CodeEditorScrollbar(QWidget* parent) : QScrollBar(Qt::Vertical, parent), m_minimap_scrollbar(nullptr)
    {
    }

    void CodeEditorScrollbar::set_minimap_scrollbar(MinimapScrollbar* scrollbar)
    {
        m_minimap_scrollbar = scrollbar;
    }

    void CodeEditorScrollbar::sliderChange(QAbstractSlider::SliderChange change)
    {
        QAbstractSlider::sliderChange(change);
        // SIGNALS BLOCKED

        if (!m_minimap_scrollbar)
            return;

        switch (change)
        {
            case QAbstractSlider::SliderRangeChange:
                m_minimap_scrollbar->set_range(minimum(), maximum());
                break;
            case QAbstractSlider::SliderValueChange:
                m_minimap_scrollbar->set_value(value());
                break;
            default:
                break;
        }
    }
}

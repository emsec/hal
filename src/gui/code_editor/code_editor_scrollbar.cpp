#include "code_editor/code_editor_scrollbar.h"

#include "code_editor/minimap_scrollbar.h"
namespace hal
{
    code_editor_scrollbar::code_editor_scrollbar(QWidget* parent) : QScrollBar(Qt::Vertical, parent), m_minimap_scrollbar(nullptr)
    {
    }

    void code_editor_scrollbar::set_minimap_scrollbar(minimap_scrollbar* scrollbar)
    {
        m_minimap_scrollbar = scrollbar;
    }

    void code_editor_scrollbar::sliderChange(QAbstractSlider::SliderChange change)
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

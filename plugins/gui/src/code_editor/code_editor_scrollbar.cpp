#include "gui/code_editor/code_editor_scrollbar.h"

#include "gui/code_editor/minimap_scrollbar.h"
namespace hal
{
    CodeEditorScrollbar::CodeEditorScrollbar(QWidget* parent) : QScrollBar(Qt::Vertical, parent), mMinimapScrollbar(nullptr)
    {
    }

    void CodeEditorScrollbar::setMinimapScrollbar(MinimapScrollbar* scrollbar)
    {
        mMinimapScrollbar = scrollbar;
    }

    void CodeEditorScrollbar::sliderChange(QAbstractSlider::SliderChange change)
    {
        QAbstractSlider::sliderChange(change);
        // SIGNALS BLOCKED

        if (!mMinimapScrollbar)
            return;

        switch (change)
        {
            case QAbstractSlider::SliderRangeChange:
                mMinimapScrollbar->setRange(minimum(), maximum());
                break;
            case QAbstractSlider::SliderValueChange:
                mMinimapScrollbar->setValue(value());
                break;
            default:
                break;
        }
    }
}

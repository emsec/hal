#include "gui/new_selection_details_widget/debug_content_widget.h"

namespace hal {
    DebugContentWidget::DebugContentWidget(QWidget *parent) : ContentWidget("Debug Widget", parent)
    {
        mContent = new LUTTableWidget(this);

        mContentLayout->addWidget(mContent);
    }

} // namespace hal
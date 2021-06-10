#include "gui/new_selection_details_widget/debug_content_widget.h"

namespace hal {
    DebugContentWidget::DebugContentWidget(QWidget *parent) : ContentWidget("Debug Widget", parent)
    {
        mContentLayout->addWidget(new BooleanFunctionTable(this));
    }

} // namespace hal
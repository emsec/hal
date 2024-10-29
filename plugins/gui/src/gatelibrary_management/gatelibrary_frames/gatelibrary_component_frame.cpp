#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_component_frame.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"

namespace hal {
    GatelibraryComponentFrame::GatelibraryComponentFrame(const QString& title, QWidget* parent)
        : QFrame(parent), mTitle(title)
    {
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        setFrameStyle(QFrame::Sunken | QFrame::Panel);
        setLineWidth(2);
        mLayout = new QFormLayout(this);
        mLayout->addRow(new GateLibraryLabel(false, mTitle, this));

        //mLayout->setSizeConstraint(QLayout::SetMinimumSize);
    }
}

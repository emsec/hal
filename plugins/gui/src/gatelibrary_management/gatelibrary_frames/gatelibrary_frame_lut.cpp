#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_lut.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

namespace hal
{
    GatelibraryFrameLut::GatelibraryFrameLut(QWidget* parent)
        : GatelibraryComponentFrame("LUT Init", parent)
    {
        mAscending                = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "Bit Order:", parent), mAscending);
    }

    void GatelibraryFrameLut::update(GateType* gt)
    {
        if(gt->has_component_of_type(GateTypeComponent::ComponentType::init))
        {
            auto lutc = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });

            if(lutc != nullptr)
            {
                mAscending->setText(QString::fromStdString(lutc->is_init_ascending() ? "Ascending" : "Descending"));
                show();
            }
            else hide();
        }
        else hide();
    }
}


#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_latch.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"

namespace hal
{
    GateLibraryFrameLatch::GateLibraryFrameLatch(QWidget* parent)
        : GatelibraryComponentFrame("Latch", parent)
    {
        mEnableOn = new GateLibraryLabel(true, " - ", this);
        mDataIn = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "Enable behaviour function:", parent), mEnableOn);
        mLayout->addRow(new GateLibraryLabel(false, "Data input function:", parent), mDataIn);

        mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    }

    void GateLibraryFrameLatch::update(GateType* gt)
    {
        if(gt->has_component_of_type(GateTypeComponent::ComponentType::latch))
        {
            auto latch = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c) {return LatchComponent::is_class_of(c);});

            if(latch != nullptr)
            {
                mEnableOn->setText(QString::fromStdString(latch->get_enable_function().to_string()));
                mDataIn->setText(QString::fromStdString(latch->get_data_in_function().to_string()));
                show();
            }
            else
                hide();
        }
        else
            hide();
    }
}


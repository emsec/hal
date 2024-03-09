#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_state.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"

namespace hal
{
    GatelibraryFrameState::GatelibraryFrameState(QWidget* parent)
        : GatelibraryComponentFrame("Internal States", parent)
    {
        mStateIdentifier              = new GateLibraryLabel(true, " - ", this);
        mNegStateIdentifier      = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "State identifier:",        parent), mStateIdentifier);
        mLayout->addRow(new GateLibraryLabel(false, "Neg. state identifier:",   parent), mNegStateIdentifier);

    }

    void GatelibraryFrameState::update(GateType* gt)
    {
        if(gt->has_component_of_type(GateTypeComponent::ComponentType::state))
        {
            auto stat = gt->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });

            if(stat != nullptr)
            {
                mStateIdentifier->setText(QString::fromStdString(stat->get_state_identifier()));
                mNegStateIdentifier->setText(QString::fromStdString(stat->get_neg_state_identifier()));
                show();
            }
            else hide();
        }
        else hide();
    }
}


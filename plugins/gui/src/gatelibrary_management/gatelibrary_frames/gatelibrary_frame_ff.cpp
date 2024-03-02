#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ff.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{

    GatelibraryFrameFF::GatelibraryFrameFF(QWidget* parent)
        : GatelibraryComponentFrame("Flip Flop", parent)
    {
        mClockPropertyLabel                = new GateLibraryLabel(true, " - ", this);
        mNextStatePropertyLabel            = new GateLibraryLabel(true, " - ", this);
        mAsynchronousResetPropertyLabel    = new GateLibraryLabel(true, " - ", this);
        mInternalStatePropertyLabel        = new GateLibraryLabel(true, " - ", this);
        mNegatedInternalStatePropertyLabel = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "Clock:",                  parent), mClockPropertyLabel);
        mLayout->addRow(new GateLibraryLabel(false, "Next state:",             parent), mNextStatePropertyLabel);
        mLayout->addRow(new GateLibraryLabel(false, "Asynchronous reset:",     parent), mAsynchronousResetPropertyLabel);
        mLayout->addRow(new GateLibraryLabel(false, "Internal state:",         parent), mInternalStatePropertyLabel);
        mLayout->addRow(new GateLibraryLabel(false, "Negated internal state:", parent), mNegatedInternalStatePropertyLabel);
    }

    void GatelibraryFrameFF::update(GateType* gt)
    {
        if (gt->has_component_of_type(GateTypeComponent::ComponentType::ff))
        {
            auto ff = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });

            if (ff != nullptr)
            {
                mClockPropertyLabel->setText(QString::fromStdString(ff->get_clock_function().to_string()));
                mNextStatePropertyLabel->setText(QString::fromStdString(ff->get_next_state_function().to_string()));
                mAsynchronousResetPropertyLabel->setText(QString::fromStdString(ff->get_async_reset_function().to_string()));

                mInternalStatePropertyLabel->setText(QString::fromStdString(gt->get_boolean_function().to_string()));

                Result<BooleanFunction> result = BooleanFunction::Not(gt->get_boolean_function(), gt->get_boolean_function().size());
                if(result.is_ok())
                    mNegatedInternalStatePropertyLabel->setText(QString::fromStdString(result.get().to_string()));
                else{
                    mNegatedInternalStatePropertyLabel->setText("ERROR");
                }
                show();
            }
            else
                hide();
        }
        else
            hide();
    }
}

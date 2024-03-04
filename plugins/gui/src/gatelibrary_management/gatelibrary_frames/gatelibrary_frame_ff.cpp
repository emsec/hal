#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ff.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{

    GatelibraryFrameFF::GatelibraryFrameFF(QWidget* parent)
        : GatelibraryComponentFrame("Flip Flop", parent)
    {
        mClockProperty                = new GateLibraryLabel(true, " - ", this);
        mNextStateProperty            = new GateLibraryLabel(true, " - ", this);
        mAsynchronousResetProperty    = new GateLibraryLabel(true, " - ", this);
        mAsynchronousSetProperty      = new GateLibraryLabel(true, " - ", this);
        mInternalStateProperty        = new GateLibraryLabel(true, " - ", this);
        mNegatedInternalStateProperty = new GateLibraryLabel(true, " - ", this);
        mInternalStateOnReset         = new GateLibraryLabel(true, " - ", this);
        mNegatedInternalStateOnReset  = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "Clock:",                   parent), mClockProperty);
        mLayout->addRow(new GateLibraryLabel(false, "Next state:",              parent), mNextStateProperty);
        mLayout->addRow(new GateLibraryLabel(false, "Asynchronous reset:",      parent), mAsynchronousResetProperty);
        mLayout->addRow(new GateLibraryLabel(false, "Asynchronous set:",        parent), mAsynchronousSetProperty);
        mLayout->addRow(new GateLibraryLabel(false, "Internal state:",          parent), mInternalStateProperty);
        mLayout->addRow(new GateLibraryLabel(false, "Negated internal state:",  parent), mNegatedInternalStateProperty);
        mLayout->addRow(new GateLibraryLabel(false, "Internal state on reset:", parent), mInternalStateOnReset);
        mLayout->addRow(new GateLibraryLabel(false, "Neg.int. state on reset:", parent), mNegatedInternalStateOnReset);
     }

    void GatelibraryFrameFF::update(GateType* gt)
    {
        if (gt->has_component_of_type(GateTypeComponent::ComponentType::ff))
        {
            auto ff = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });

            if (ff != nullptr)
            {
                mClockProperty->setText(QString::fromStdString(ff->get_clock_function().to_string()));
                mNextStateProperty->setText(QString::fromStdString(ff->get_next_state_function().to_string()));

                if (ff->get_async_reset_function().is_empty())
                {
                    mAsynchronousResetProperty->setText("N/A");
                    mAsynchronousResetProperty->setValue(false);
                }
                else
                {
                    mAsynchronousResetProperty->setText(QString::fromStdString(ff->get_async_reset_function().to_string()));
                    mAsynchronousResetProperty->setValue(true);
                }

                if (ff->get_async_set_function().is_empty())
                {
                    mAsynchronousResetProperty->setText("N/A");
                    mAsynchronousResetProperty->setValue(false);
                }
                else
                {
                    mAsynchronousResetProperty->setText(QString::fromStdString(ff->get_async_set_function().to_string()));
                    mAsynchronousResetProperty->setValue(true);
                }

                mInternalStateProperty->setText(QString::fromStdString(gt->get_boolean_function().to_string()));

                Result<BooleanFunction> result = BooleanFunction::Not(gt->get_boolean_function(), gt->get_boolean_function().size());
                if(result.is_ok())
                {
                    mNegatedInternalStateProperty->setText(QString::fromStdString(result.get().to_string()));
                    mNegatedInternalStateProperty->setValue(true);
                }
                else
                {
                    mNegatedInternalStateProperty->setText("ERROR");
                    mNegatedInternalStateProperty->setValue(false);
                }

                auto [stateBeh,negStateBeh] = ff->get_async_set_reset_behavior();
                        mInternalStateOnReset->setText(QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(stateBeh)));
                        mInternalStateOnReset->setText(QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(negStateBeh)));

                show();
            }
            else
                hide();
        }
        else
            hide();
    }
}


#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_flip_flop.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{

    GateLibraryTabFlipFlop::GateLibraryTabFlipFlop(QWidget* parent) : GateLibraryTabInterface(parent)
    {
        mFormLayout = new QFormLayout(parent);

        mClockLabel = new QLabel("Clock", parent);
        mNextStateLabel = new QLabel("Next state", parent);
        mAsynchronousResetLabel = new QLabel("Asynchronous reset", parent);
        mInternalStateLabel = new QLabel("Internal state", parent);
        mNegatedInternalStateLabel = new QLabel("Negated internal state", parent);

        mClockPropertyLabel = new QLabel(" - ", parent);
        mNextStatePropertyLabel = new QLabel(" - ", parent);
        mAsynchronousResetPropertyLabel = new QLabel(" - ", parent);
        mInternalStatePropertyLabel = new QLabel(" - ", parent);
        mNegatedInternalStatePropertyLabel = new QLabel(" - ", parent);

        mFormLayout->addRow(mClockLabel, mClockPropertyLabel);
        mFormLayout->addRow( mNextStateLabel, mNextStatePropertyLabel);
        mFormLayout->addRow(mAsynchronousResetLabel, mAsynchronousResetPropertyLabel);
        mFormLayout->addRow(mInternalStateLabel, mInternalStatePropertyLabel);
        mFormLayout->addRow(mNegatedInternalStateLabel, mNegatedInternalStatePropertyLabel);

        setLayout(mFormLayout);

    }

    void GateLibraryTabFlipFlop::update(GateType* gate)
    {
        if(!gate || !gate->has_component_of_type(GateTypeComponent::ComponentType::ff)){
            //TODO make default look
            mClockPropertyLabel->setText("-");
            mNextStatePropertyLabel->setText("-");
            mAsynchronousResetPropertyLabel->setText("-");
            mInternalStatePropertyLabel->setText("-");
            mNegatedInternalStatePropertyLabel->setText("-");

            return;
        }

        auto ff = gate->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });

        if (ff != nullptr)
        {
            mClockPropertyLabel->setText(QString::fromStdString(ff->get_clock_function().to_string()));
            mNextStatePropertyLabel->setText(QString::fromStdString(ff->get_next_state_function().to_string()));
            mAsynchronousResetPropertyLabel->setText(QString::fromStdString(ff->get_async_reset_function().to_string()));

            mInternalStatePropertyLabel->setText(QString::fromStdString(gate->get_boolean_function().to_string()));

            Result<BooleanFunction> result = BooleanFunction::Not(gate->get_boolean_function(), gate->get_boolean_function().size());
            if(result.is_ok())
                mNegatedInternalStatePropertyLabel->setText(QString::fromStdString(result.get().to_string()));
            else{
                mNegatedInternalStatePropertyLabel->setText("ERROR");
            }

        }
        else{
            qDebug() << "could not cast component to FFComponent";
            return;
        }

    }

}

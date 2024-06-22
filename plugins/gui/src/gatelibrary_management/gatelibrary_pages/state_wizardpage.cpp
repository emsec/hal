#include "gui/gatelibrary_management/gatelibrary_pages/state_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"

namespace hal
{
    StateWizardPage::StateWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("State");
        setSubTitle("Enter parameters for state component");
        mLayout = new QGridLayout(this);

        mStateIdentifier = new QLineEdit(this);
        mNegStateIdentifier = new QLineEdit(this);

        mLabStateIdentifier = new QLabel("State identifier: ");
        mLabNegStateIdentifier = new QLabel("Negative state identifier: ");

        mLayout->addWidget(mLabStateIdentifier, 0, 0);
        mLayout->addWidget(mStateIdentifier, 0, 1);
        mLayout->addWidget(mLabNegStateIdentifier, 1, 0);
        mLayout->addWidget(mNegStateIdentifier, 1, 1);

        setLayout(mLayout);
    }

    void StateWizardPage::setData(GateType *gate){
        if(gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::state))
        {
            auto stat = gate->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });

            if(stat != nullptr)
            {
                mStateIdentifier->setText(QString::fromStdString(stat->get_state_identifier()));
                mNegStateIdentifier->setText(QString::fromStdString(stat->get_neg_state_identifier()));
            }
        }
    }
}

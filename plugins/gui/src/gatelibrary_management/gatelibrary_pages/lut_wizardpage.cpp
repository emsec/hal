#include "gui/gatelibrary_management/gatelibrary_pages/lut_wizardpage.h"
#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

namespace hal
{
    LUTWizardPage::LUTWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("LUT Init");
        setSubTitle("Enter parameters for LUT component");
        mLayout = new QGridLayout(this);

        mAscending = new QCheckBox(this);

        mLabAscending = new QLabel("Bit order ascending: ");

        mLayout->addWidget(mLabAscending, 0, 0);
        mLayout->addWidget(mAscending, 0, 1);

        setLayout(mLayout);

        connect(mAscending, &QCheckBox::stateChanged, this, &LUTWizardPage::completeChanged);
    }

    void LUTWizardPage::setData(GateType *gate){
        if(gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::init))
        {
            auto lutc = gate->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });

            if(lutc != nullptr) mAscending->setChecked(lutc->is_init_ascending());
        }

    }
}

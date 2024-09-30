#include "gui/gatelibrary_management/gatelibrary_pages/ram_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"

namespace hal
{
    RAMWizardPage::RAMWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("RAM");
        setSubTitle("Enter parameters for RAM component");
        mLayout = new QGridLayout(this);

        mBitSize = new QLineEdit(this);

        mLabBitSize = new QLabel("RAM bit size: ");

        mLayout->addWidget(mLabBitSize, 0, 0);
        mLayout->addWidget(mBitSize, 0, 1);

        setLayout(mLayout);

        connect(mBitSize, &QLineEdit::textChanged, this, &RAMWizardPage::completeChanged);
    }

    void RAMWizardPage::setData(GateType *gate){
        if(gate->has_component_of_type(GateTypeComponent::ComponentType::ram))
        {
            auto ram = gate->get_component_as<RAMComponent>([](const GateTypeComponent* c) {return RAMComponent::is_class_of(c);});

            if(ram != nullptr)
            {
                mBitSize->setText(QString::fromStdString(std::to_string(ram->get_bit_size())));
            }
        }

    }
}

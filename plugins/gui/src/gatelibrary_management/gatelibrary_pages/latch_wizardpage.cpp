#include "gui/gatelibrary_management/gatelibrary_pages/latch_wizardpage.h"
#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"

namespace hal
{
    LatchWizardPage::LatchWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Latch");
        setSubTitle("Enter parameters for latch component");
        mLayout = new QGridLayout(this);

        mDataIn = new QLineEdit(this);
        mEnableOn = new QLineEdit(this);

        mLabDataIn = new QLabel("Data input function: ");
        mLabEnableOn = new QLabel("Enable behaviour function: ");

        mLayout->addWidget(mLabDataIn, 0, 0);
        mLayout->addWidget(mDataIn, 0, 1);
        mLayout->addWidget(mLabEnableOn, 1, 0);
        mLayout->addWidget(mEnableOn, 1, 1);

        setLayout(mLayout);

    }

//    int LatchWizardPage::nextId() const
//    {
//        auto parentWizard = wizard();
//        if(!parentWizard)
//            return -1;
//        return static_cast<GateLibraryWizard*>(parentWizard)->getNextPageId(GateLibraryWizard::Latch);
//    }

    void LatchWizardPage::setData(GateType *gate){
        if(gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::latch))
        {
            auto latch = gate->get_component_as<LatchComponent>([](const GateTypeComponent* c) {return LatchComponent::is_class_of(c);});

            if(latch != nullptr)
            {
                mEnableOn->setText(QString::fromStdString(latch->get_enable_function().to_string()));
                mDataIn->setText(QString::fromStdString(latch->get_data_in_function().to_string()));
            }
        }

    }
}

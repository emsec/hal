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
        mAReset = new QLineEdit(this);
        mASet = new QLineEdit(this);
        mIntState = new QLineEdit(this);
        mNegIntState = new QLineEdit(this);

        mLabDataIn = new QLabel("Data input function: ", this);
        mLabEnableOn = new QLabel("Enable behaviour function: ", this);
        mLabAReset = new QLabel("Asynchronous reset: ", this);
        mLabASet = new QLabel("Asynchronous set: ", this);
        mLabIntState = new QLabel("Set+Reset -> internal state: ", this);
        mLabNegIntState = new QLabel("Set+Reset -> neg. int. state:", this);

        mLayout->addWidget(mLabDataIn, 0, 0);
        mLayout->addWidget(mDataIn, 0, 1);
        mLayout->addWidget(mLabEnableOn, 1, 0);
        mLayout->addWidget(mEnableOn, 1, 1);
        mLayout->addWidget(mLabAReset, 2, 0);
        mLayout->addWidget(mAReset, 2, 1);
        mLayout->addWidget(mLabASet, 3, 0);
        mLayout->addWidget(mASet, 3, 1);
        mLayout->addWidget(mLabIntState, 4, 0);
        mLayout->addWidget(mIntState, 4, 1);
        mLayout->addWidget(mLabNegIntState, 5, 0);
        mLayout->addWidget(mNegIntState, 5, 1);

        mAReset->setDisabled(true);
        mASet->setDisabled(true);

        setLayout(mLayout);

    }

    void LatchWizardPage::setData(GateType *gate){
        if(gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::latch))
        {
            auto latch = gate->get_component_as<LatchComponent>([](const GateTypeComponent* c) {return LatchComponent::is_class_of(c);});

            if(latch != nullptr)
            {
                mEnableOn->setText(QString::fromStdString(latch->get_enable_function().to_string()));
                mDataIn->setText(QString::fromStdString(latch->get_data_in_function().to_string()));

                if (latch->get_async_reset_function().is_empty()) mAReset->setText("N/A");
                else {
                    mAReset->setDisabled(false);
                    mAReset->setText(QString::fromStdString(latch->get_async_reset_function().to_string()));
                }
                if (latch->get_async_set_function().is_empty()) mASet->setText("N/A");
                else {
                    mASet->setDisabled(false);
                    mASet->setText(QString::fromStdString(latch->get_async_set_function().to_string()));
                }

                auto [stateBeh,negStateBeh] = latch->get_async_set_reset_behavior();
                if (stateBeh == AsyncSetResetBehavior::undef) mIntState->setText("undefined");
                else mIntState->setText(QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(stateBeh)));

                if (negStateBeh == AsyncSetResetBehavior::undef) mNegIntState->setText("undefined");
                else mNegIntState->setText(QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(negStateBeh)));
            }
        }

    }
}

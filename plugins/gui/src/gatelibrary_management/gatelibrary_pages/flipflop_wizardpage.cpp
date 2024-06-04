#include "gui/gatelibrary_management/gatelibrary_pages/flipflop_wizardpage.h"

#include <QDebug>
#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{
    FlipFlopWizardPage::FlipFlopWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 3: Flip Flop");
        setSubTitle("Enter parameters for flip flop component");
        mLayout = new QGridLayout(this);
        mTabWidget = new QTabWidget(this);

        mClock = new QLineEdit(this);
        mNextState = new QLineEdit(this);
        mAReset = new QLineEdit(this);
        mASet = new QLineEdit(this);
        mIntState = new QLineEdit(this);
        mNegIntState = new QLineEdit(this);

        mLabClock = new QLabel("Clock: ", this);
        mLabNextState = new QLabel("Next state: ", this);
        mLabAReset = new QLabel("Asynchronous reset: ", this);
        mLabASet = new QLabel("Asynchronous set: ", this);
        mLabIntState = new QLabel("Set+Reset -> internal state: ", this);
        mLabNegIntState = new QLabel("Set+Reset -> neg. int. state:", this);

        mLayout->addWidget(mLabClock, 0, 0);
        mLayout->addWidget(mClock, 0, 1);
        mLayout->addWidget(mLabNextState, 1, 0);
        mLayout->addWidget(mNextState, 1, 1);
        mLayout->addWidget(mLabAReset, 2, 0);
        mLayout->addWidget(mAReset, 2, 1);
        mLayout->addWidget(mLabASet, 3, 0);
        mLayout->addWidget(mASet, 3, 1);
        mLayout->addWidget(mLabIntState, 4, 0);
        mLayout->addWidget(mIntState, 4, 1);
        mLayout->addWidget(mLabNegIntState, 5, 0);
        mLayout->addWidget(mNegIntState, 5, 1);

        //TODO:
        //mTabWidget->addTab(mStateTableTab, "State Table");

        mAReset->setDisabled(true);
        mASet->setDisabled(true);

        setLayout(mLayout);

    }

    void FlipFlopWizardPage::initializePage()
    {
        //qInfo() << field("name").toString();
        //qInfo() << field("properties").toInt();
    }

    void FlipFlopWizardPage::setData(GateType *gate){
        if (gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::ff))
        {
            auto ff = gate->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });

            if (ff != nullptr)
            {
                mClock->setText(QString::fromStdString(ff->get_clock_function().to_string()));
                mNextState->setText(QString::fromStdString(ff->get_next_state_function().to_string()));

                if (ff->get_async_reset_function().is_empty()) mAReset->setText("N/A");
                else {
                    mAReset->setDisabled(false);
                    mAReset->setText(QString::fromStdString(ff->get_async_reset_function().to_string()));
                }

                if (ff->get_async_set_function().is_empty()) mASet->setText("N/A");
                else {
                    mASet->setDisabled(false);
                    mASet->setText(QString::fromStdString(ff->get_async_set_function().to_string()));
                }

                auto [stateBeh,negStateBeh] = ff->get_async_set_reset_behavior();
                if (stateBeh == AsyncSetResetBehavior::undef) mIntState->setText("undefined");
                else mIntState->setText(QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(stateBeh)));

                if (negStateBeh == AsyncSetResetBehavior::undef) mNegIntState->setText("undefined");
                else mNegIntState->setText(QString::fromStdString(enum_to_string<AsyncSetResetBehavior>(negStateBeh)));
            }
        }
    }
}

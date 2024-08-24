#include "gui/gatelibrary_management/gatelibrary_pages/flipflop_wizardpage.h"

#include <QDebug>
#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"

namespace hal
{
    FlipFlopWizardPage::FlipFlopWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Flip Flop");
        setSubTitle("Enter parameters for flip flop component");
        mLayout = new QGridLayout(this);
        mTabWidget = new QTabWidget(this);

        mClock = new BooleanFunctionEdit(mLegVars, this);
        mNextState = new BooleanFunctionEdit(mLegVars, this);
        mAReset = new BooleanFunctionEdit(mLegVars, this);
        mASet = new BooleanFunctionEdit(mLegVars, this);
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

        setLayout(mLayout);

        connect(mClock, &QLineEdit::textChanged, this, &FlipFlopWizardPage::handleTextChanged);
        connect(mNextState, &QLineEdit::textChanged, this, &FlipFlopWizardPage::handleTextChanged);
        connect(mAReset, &QLineEdit::textChanged, this, &FlipFlopWizardPage::handleTextChanged);
        connect(mASet, &QLineEdit::textChanged, this, &FlipFlopWizardPage::handleTextChanged);
        connect(mIntState, &QLineEdit::textChanged, this, &FlipFlopWizardPage::handleTextChanged);
        connect(mNegIntState, &QLineEdit::textChanged, this, &FlipFlopWizardPage::handleTextChanged);
    }

    void FlipFlopWizardPage::initializePage()
    {
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        QList<PinItem*> inputPins = mWizard->mPinModel->getInputPins();

        mLegVars.clear();
        for (PinItem* pi : inputPins)
            mLegVars.insert(pi->getName().toStdString());

        mClock->setLegalVariables(mLegVars);
        mNextState->setLegalVariables(mLegVars);
        mAReset->setLegalVariables(mLegVars);
        mASet->setLegalVariables(mLegVars);

        if(mWizard->statePage->mNegStateIdentifier->text().isEmpty())
        {
            mNegIntState->clear();
            mNegIntState->setDisabled(true);
        }
        else mNegIntState->setDisabled(false);

        bool clock = false;
        bool set = false;
        bool reset = false;

        QList<PinItem*> pingroups = mWizard->getPingroups();
        for (PinItem* pg : pingroups) {
            for(BaseTreeItem* it: pg->getChildren())
            {
                PinItem* p = static_cast<PinItem*>(it);
                if(!clock && p->getPinType() == PinType::clock) {
                    mClock->setText(p->getName()); //name of first pin with type clock
                    clock = true;
                }
                if(!reset && p->getPinType() == PinType::reset) {
                    mAReset->setText(p->getName()); //name of first pin with type reset
                    reset = true;
                }
                if(!set && p->getPinType() == PinType::set) {
                    mASet->setText(p->getName()); //name of first pin with type set
                    set = true;
                }
            }
        }

        if(!clock) mClock->clear();
        if(!reset) mAReset->clear();
        if(!set) mASet->clear();

        if(mASet->text().isEmpty() || mAReset->text().isEmpty())
        {
            mIntState->clear();
            mNegIntState->clear();
            mIntState->setDisabled(true);
            mNegIntState->setDisabled(true);
        }
        else
        {
            mIntState->setDisabled(false);
            mNegIntState->setDisabled(false);
        }

        Q_EMIT completeChanged();
    }

    void FlipFlopWizardPage::handleTextChanged(const QString& text){
        Q_UNUSED(text);
        if(mASet->text().isEmpty() || mAReset->text().isEmpty())
        {
            mIntState->clear();
            mNegIntState->clear();
            mIntState->setDisabled(true);
            mNegIntState->setDisabled(true);
        }
        else
        {
            mIntState->setDisabled(false);
            mNegIntState->setDisabled(false);
        }

        Q_EMIT completeChanged();
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


    bool FlipFlopWizardPage::isComplete() const{
        if(mClock->text().isEmpty() || mNextState->text().isEmpty()) return false;
        if(!mClock->isValid() || !mNextState->isValid() || mAReset->state() == "Invalid" || mASet->state() == "Invalid") return false;
        if(!mASet->text().isEmpty() && !mAReset->text().isEmpty())
        {
            if(mIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::H) &&
                    mIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::L) &&
                    mIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::N) &&
                    mIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::T) &&
                    mIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::X))
                return false;
            if(mNegIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::H) &&
                    mNegIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::L) &&
                    mNegIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::N) &&
                    mNegIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::T) &&
                    mNegIntState->text().toStdString() != enum_to_string(AsyncSetResetBehavior::X))
                return false;
        }
        return true;
    }
}

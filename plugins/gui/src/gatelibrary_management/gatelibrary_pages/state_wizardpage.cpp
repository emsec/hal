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

        connect(mStateIdentifier, &QLineEdit::textChanged, this, &StateWizardPage::handleTextChanged);
        connect(mNegStateIdentifier, &QLineEdit::textChanged, this, &StateWizardPage::handleNegTextChanged);

        QRegExp rx("[A-Z]([A-Z]|\\d|_)*");
        mValidator = new QRegExpValidator(rx, this);
        mStateIdentifier->setValidator(mValidator);
        mNegStateIdentifier->setValidator(mValidator);
    }

    void StateWizardPage::initializePage()
    {
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mPinGroups = mWizard->getPingroups();
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

    void StateWizardPage::handleTextChanged(const QString &text)
    {
        mNegStateIdentifier->setText(QString("%1N").arg(mStateIdentifier->text()));
        Q_UNUSED(text);
        Q_EMIT completeChanged();
    }

    void StateWizardPage::handleNegTextChanged(const QString &text)
    {
        Q_UNUSED(text);
        Q_EMIT completeChanged();
    }

    bool StateWizardPage::isComplete() const
    {
        if(mStateIdentifier->text().isEmpty()) return false;
        for (auto pingroup : mPinGroups) { //check if pins name is used as a state identifier name
            QString groupName = pingroup->getName();
            if(mStateIdentifier->text() == groupName || mNegStateIdentifier->text() == groupName) return false;
            for(auto it : pingroup->getChildren())
            {
                PinItem* pin = static_cast<PinItem*>(it);
                QString pinName = pin->getName();
                if(mStateIdentifier->text() == pinName || mNegStateIdentifier->text() == pinName) return false;
            }
        }
        return true;
    }
}

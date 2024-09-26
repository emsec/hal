#include "gui/gatelibrary_management/gatelibrary_pages/init_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"

namespace hal
{
    InitWizardPage::InitWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Init from Data Field");
        setSubTitle("Enter parameters for init component");
        mLayout = new QGridLayout(this);

        mCategory = new QLineEdit(this);
        mIdentifiers = new QTextEdit(this);

        mLabCategory = new QLabel("Category: ");
        mLabIdentifiers = new QLabel("Identifiers: ");

        mLayout->addWidget(mLabCategory, 0, 0);
        mLayout->addWidget(mCategory, 0, 1);
        mLayout->addWidget(mLabIdentifiers, 1, 0);
        mLayout->addWidget(mIdentifiers, 1, 1);

        setLayout(mLayout);

        connect(mCategory, &QLineEdit::textChanged, this, &InitWizardPage::handleTextChanged);
        connect(mIdentifiers, &QTextEdit::textChanged, this, &InitWizardPage::handleTextEditChanged);
    }

    void InitWizardPage::initializePage()
    {
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mWizard->mEditMode = true;
    }

    void InitWizardPage::setData(GateType *gate){
        if(gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::init))
        {
            auto init = gate->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });

            if(init != nullptr)
            {
                mCategory->setText(QString::fromStdString(init->get_init_category()));
                QString ids = "";

                bool first = true;
                for (std::string i : init->get_init_identifiers()) {
                    if(first)
                        first = false;
                    else
                        ids.append("\n");

                    ids.append(QString::fromStdString(i));
                }
                mIdentifiers->setText(ids);
            }
        }
    }

    void InitWizardPage::handleTextChanged(const QString& txt)
    {
        Q_UNUSED(txt);
        mWizard = static_cast<GateLibraryWizard*>(wizard());

        //explicitly needed here because isComplete() is called
        //before mWasEdited is changed in the wizard
        if(mWizard->mEditMode) mWizard->mWasEdited = true;

        Q_EMIT completeChanged();
    }

    void InitWizardPage::handleTextEditChanged()
    {
        mWizard = static_cast<GateLibraryWizard*>(wizard());

        //explicitly needed here because isComplete() is called
        //before mWasEdited is changed in the wizard
        if(mWizard->mEditMode) mWizard->mWasEdited = true;

        Q_EMIT completeChanged();
    }

    bool InitWizardPage::isComplete() const
    {
        if(isFinalPage() && !mWizard->mWasEdited) return false;
        mWizard->mEditMode = false;
        return true;
    }
}

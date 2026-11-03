#include "gui/gatelibrary_management/gatelibrary_pages/bool_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/pin_model/pin_item.h"

namespace hal
{
    const char* BooleanFunctionEdit::STATE_EMPTY = "Empty";
    const char* BooleanFunctionEdit::STATE_VALID = "Valid";
    const char* BooleanFunctionEdit::STATE_INVALID = "Invalid";

    BooleanFunctionEdit::BooleanFunctionEdit(std::set<std::string> &legalVar, QWidget *parent)
        : QLineEdit(parent), mState(STATE_VALID), mLegalVariables(legalVar)
    {
        connect(this, &QLineEdit::textChanged, this, &BooleanFunctionEdit::handleEditingFinished);
        connect(this, &BooleanFunctionEdit::legalVariablesChanged, this, &BooleanFunctionEdit::handleEditingFinished);

        setState(STATE_EMPTY); // do an active transition to enforce style
    }

    void BooleanFunctionEdit::setState(const QString &s)
    {
        if (s == mState) return;
        mState = s;
        Q_EMIT stateChanged(s);
        QStyle* sty = style();

        sty->unpolish(this);
        sty->polish(this);
    }

    void BooleanFunctionEdit::setLegalVariables(std::set<std::string> &legalVar)
    {
        mLegalVariables = legalVar;
        Q_EMIT legalVariablesChanged(legalVar);
    }

    void BooleanFunctionEdit::handleEditingFinished()
    {
        if (text().isEmpty())
        {
            setState(STATE_EMPTY);
            return;
        }

        QString nextState = STATE_VALID;  // think positive

        auto bfres = BooleanFunction::from_string(text().toStdString());
        if(bfres.is_error())
            nextState = STATE_INVALID;
        else
        {
            BooleanFunction bf = bfres.get();
            std::set<std::string> var_names = bf.get_variable_names();
            std::set<std::string> leg = mLegalVariables;
            for(std::string vname : var_names)
            {
                if (mLegalVariables.find(vname) == mLegalVariables.end())
                {
                    nextState = STATE_INVALID;
                    break;
                }
            }
        }
        if (mState != nextState)
            setState(nextState);

    }
//--------------------------------------------
    BoolWizardPage::BoolWizardPage(QWidget* parent)
        : QWizardPage(parent)
    {
        setTitle("Boolean functions");
        setSubTitle("Enter the boolean functions");
        mLayout = new QGridLayout(this);
    }

    void BoolWizardPage::initializePage(){
        mWizard = static_cast<GateLibraryWizard*>(wizard());

        std::set<std::string> legVars;
        for (PinItem* pi : mWizard->mPinModel->getInputPins())
            legVars.insert(pi->getName().toStdString());

        for (PinItem* pi : mWizard->mPinModel->getInternalPins())
            legVars.insert(pi->getName().toStdString());

        if(mWizard->generalInfoPage->getProperties().contains(GateTypeProperty::ff)
                || mWizard->generalInfoPage->getProperties().contains(GateTypeProperty::latch))
        {
            legVars.insert(mWizard->statePage->mStateIdentifier->text().toStdString());
            legVars.insert(mWizard->statePage->mNegStateIdentifier->text().toStdString());
        }

        if (!mEditFunctions.isEmpty())
        {
            for (BooleanFunctionEdit* bfe : mEditFunctions)
                delete bfe;
            mEditFunctions.clear();
        }

        std::unordered_map<std::string, BooleanFunction> boolFunctions;
        if(mGate != nullptr)
            boolFunctions = mGate->get_boolean_functions();

        int boolFuncCnt = 0;
        for(PinItem* pin : mWizard->mPinModel->getOutputPins())
        {
            if(mWizard->isDefinedByLut(pin))
            {
                //the LUT defines this pin already, so it may be used as input of the remaining functions
                legVars.insert(pin->getName().toStdString());
                continue;
            }

            QLabel* label = new QLabel(pin->getName(), this);
            BooleanFunctionEdit* lineEdit = new BooleanFunctionEdit(legVars, this);
            mLayout->addWidget(label, boolFuncCnt, 0);
            mLayout->addWidget(lineEdit, boolFuncCnt, 1);

            if(auto bf = boolFunctions.find(pin->getName().toStdString()); bf != boolFunctions.end())
            {
                lineEdit->setText(QString::fromStdString(bf->second.to_string()));
            }
            connect(lineEdit, &BooleanFunctionEdit::stateChanged,this,&BoolWizardPage::handleStateChanged);
            connect(lineEdit, &BooleanFunctionEdit::textChanged, this, &BoolWizardPage::handleTextChanged);
            mEditFunctions.append(lineEdit);
            mOutputPins.append(pin->getName());

            //an output pin whose function is defined above may be used as input of the functions below it
            legVars.insert(pin->getName().toStdString());
            boolFuncCnt++;
        }

        setLayout(mLayout);
        Q_EMIT completeChanged();
        mWizard->mEditMode = true;
    }

    void BoolWizardPage::setData(GateType *gate)
    {
        mGate = gate;
    }

    void BoolWizardPage::handleStateChanged(const QString& stat)
    {
        Q_UNUSED(stat);
        Q_EMIT completeChanged();
        if(mWizard->mEditMode) Q_EMIT hasChanged();
    }

    void BoolWizardPage::handleTextChanged(const QString& txt)
    {
        Q_UNUSED(txt);

        //explicitly needed here because isComplete() is called
        //before mWasEdited is changed in the wizard
        //

        if(mWizard->mEditMode)
        {
            mWizard->mWasEdited = true;
            Q_EMIT hasChanged();
        }
        Q_EMIT completeChanged();
    }

    bool BoolWizardPage::isComplete() const
    {
        for(BooleanFunctionEdit* lineEdit : mEditFunctions)
        {
            if(!lineEdit->isValid())
            {
                return false;
            }
        }
        if(isFinalPage() && !mWizard->mWasEdited) return false;
        mWizard->mEditMode = false;
        return true;
    }

    std::unordered_map<std::string, BooleanFunction> BoolWizardPage::getBoolFunctions(){
        std::unordered_map<std::string, BooleanFunction> retval;
        for(int i = 0; i<mEditFunctions.length(); i++)
        {
            auto bfres = BooleanFunction::from_string(mEditFunctions[i]->text().toStdString());
            if(bfres.is_error())
                continue;
            else
                retval.insert({mOutputPins[i].toStdString(), bfres.get()});
        }
        return retval;
    }

}

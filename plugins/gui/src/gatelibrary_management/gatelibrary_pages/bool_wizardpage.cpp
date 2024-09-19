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
        QList<PinItem*> pinGroups = mWizard->getPingroups();

        QList<PinItem*> inputPins = mWizard->mPinModel->getInputPins();
        QList<PinItem*> outputPins = mWizard->mPinModel->getOutputPins();
        std::set<std::string> legVars;
        for (PinItem* pi : inputPins)
            legVars.insert(pi->getName().toStdString());

        if (!mEditFunctions.isEmpty())
        {
            for (BooleanFunctionEdit* bfe : mEditFunctions)
                delete bfe;
            mEditFunctions.clear();
        }

        if(mGate != nullptr){
            std::unordered_map<std::string, BooleanFunction> boolFunctions = mGate->get_boolean_functions();
            auto list = QList<QPair<QString, BooleanFunction>>();
            int boolFuncCnt = 0;

            for(PinItem* pi : outputPins)
            {
                QLabel* label = new QLabel(pi->getName());
                BooleanFunctionEdit* lineEdit;
                if(mWizard->generalInfoPage->getProperties().contains(GateTypeProperty::ff)
                        || mWizard->generalInfoPage->getProperties().contains(GateTypeProperty::latch))
                {
                    legVars.insert(mWizard->statePage->mStateIdentifier->text().toStdString());
                    legVars.insert(mWizard->statePage->mNegStateIdentifier->text().toStdString());
                }
                lineEdit = new BooleanFunctionEdit(legVars, this);
                mLayout->addWidget(label, boolFuncCnt, 0);
                mLayout->addWidget(lineEdit, boolFuncCnt, 1);

                if(auto bf = boolFunctions.find(pi->getName().toStdString()); bf != boolFunctions.end())
                {
                    lineEdit->setText(QString::fromStdString(bf->second.to_string()));
                }
                connect(lineEdit,&BooleanFunctionEdit::stateChanged,this,&BoolWizardPage::handleStateChanged);
                mEditFunctions.append(lineEdit);
                mOutputPins.append(label->text());
                boolFuncCnt++;
            }
        }
        else{
            if(!pinGroups.empty())
            {
                int rowCount = 0;
                for(PinItem* pinGroup : pinGroups){
                    if(pinGroup->getItemType() != PinItem::TreeItemType::GroupCreator && pinGroup->getDirection() == PinDirection::output){
                        for(auto item : pinGroup->getChildren())
                        {
                            PinItem* pin = static_cast<PinItem*>(item);
                            if(pin->getItemType() != PinItem::TreeItemType::PinCreator){
                                QLabel* label = new QLabel(pin->getName());
                                QString name = label->text();
                                BooleanFunctionEdit* lineEdit;

                                if(mWizard->generalInfoPage->getProperties().contains(GateTypeProperty::ff) ||
                                        mWizard->generalInfoPage->getProperties().contains(GateTypeProperty::latch))
                                {
                                    legVars.insert(mWizard->statePage->mStateIdentifier->text().toStdString());
                                    legVars.insert(mWizard->statePage->mNegStateIdentifier->text().toStdString());
                                }
                                lineEdit = new BooleanFunctionEdit(legVars, this);

                                mLayout->addWidget(label, rowCount, 0);
                                mLayout->addWidget(lineEdit, rowCount, 1);
                                connect(lineEdit,&BooleanFunctionEdit::stateChanged,this,&BoolWizardPage::handleStateChanged);
                                mEditFunctions.append(lineEdit);
                                mOutputPins.append(label->text());
                                rowCount++;
                            }
                        }
                    }
                }
            }
        }

        setLayout(mLayout);
    }

    void BoolWizardPage::setData(GateType *gate)
    {
        mGate = gate;
    }

    void BoolWizardPage::handleStateChanged(const QString& stat)
    {
        Q_UNUSED(stat);
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
        if(!mWizard->mWasEdited) return false;
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

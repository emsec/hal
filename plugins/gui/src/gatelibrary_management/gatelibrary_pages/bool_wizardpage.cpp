#include "gui/gatelibrary_management/gatelibrary_pages/bool_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/pin_model/pin_item.h"

namespace hal
{
    BoolWizardPage::BoolWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 4: Boolean functions");
        setSubTitle("TODO: subtitle");
        mLayout = new QGridLayout(this);
    }

    void BoolWizardPage::initializePage(){
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        QList<PinItem*> pinGroups = mWizard->getPingroups();

        if(mGate != nullptr){
            auto boolFunctions = mGate->get_boolean_functions();
            auto list = QList<QPair<QString, BooleanFunction>>();
            int boolFuncCnt = 0;

            for(std::pair<const std::basic_string<char>, BooleanFunction> bf : boolFunctions){
                QLabel* label = new QLabel(QString::fromStdString(bf.first));
                QLineEdit* lineEdit = new QLineEdit(this);
                mLayout->addWidget(label, boolFuncCnt, 0);
                mLayout->addWidget(lineEdit, boolFuncCnt, 1);
                lineEdit->setText(QString::fromStdString(bf.second.to_string()));
                boolFuncCnt++;
            }
        }
        else{
            if(!pinGroups.empty())
            {
                int rowCount = 0;
                for(PinItem* pinGroup : pinGroups){
                    if(pinGroup->getItemType() != PinItem::TreeItemType::GroupCreator && pinGroup->getDirection() == "output"){
                        for(auto item : pinGroup->getChildren())
                        {
                            PinItem* pin = static_cast<PinItem*>(item);
                            if(pin->getItemType() != PinItem::TreeItemType::PinCreator){
                                QLabel* label = new QLabel(pin->getName());
                                QString name = label->text();
                                QLineEdit* lineEdit = new QLineEdit(this);
                                mLayout->addWidget(label, rowCount, 0);
                                mLayout->addWidget(lineEdit, rowCount, 1);
                                rowCount++;
                            }
                        }

                    }
                }
            }
        }

        setLayout(mLayout);
    }

    void BoolWizardPage::setData(GateType *gate){
        mGate = gate;
    }

    bool BoolWizardPage::validatePage(){

        int rowCount = 0;
        QList<PinItem*> inputPins = mWizard->mPinModel->getInputPins();
        QList<PinItem*> outputPins = mWizard->mPinModel->getOutputPins();
        while(mLayout->itemAtPosition(rowCount, 1) != nullptr){
            QLabel* label = static_cast<QLabel*>(mLayout->itemAtPosition(rowCount, 1)->widget());
            auto bfres = BooleanFunction::from_string(label->text().toStdString());
            if(bfres.is_error()) return false;
            BooleanFunction bf = bfres.get();
            std::set<std::string> names = bf.get_variable_names();
            for(auto item : inputPins)
            {
                if(names.find(item->getName().toStdString()) == names.end()) return false;
            }
            rowCount++;
        }
        return true;
    }
}

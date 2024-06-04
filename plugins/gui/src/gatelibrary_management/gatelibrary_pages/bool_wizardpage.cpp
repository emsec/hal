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
}

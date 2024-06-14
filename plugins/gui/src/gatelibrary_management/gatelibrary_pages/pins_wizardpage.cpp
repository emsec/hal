#include "gui/gatelibrary_management/gatelibrary_pages/pins_wizardpage.h"

#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include <gui/pin_model/pin_delegate.h>

namespace hal
{
    PinsWizardPage::PinsWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Pins and Pingroups");
        setSubTitle("Edit pins and pingroups");
        mLayout = new QGridLayout(this);

        mPinTab = new GateLibraryTabPin(this, true);
        mDelBtn = new QPushButton("Delete", this);


        mLayout->addWidget(mDelBtn, 1, 0);
        mLayout->addWidget(mPinTab, 0, 0, 1, 2);

        connect(mDelBtn, &QPushButton::clicked, this, &PinsWizardPage::handleDeleteClicked);

        setLayout(mLayout);
    }

    void PinsWizardPage::initializePage(){

        //2
        //mPinTab = mWizard->mPinTab;
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mPinModel = mPinTab->getPinModel();
        mWizard->mPinModel = mPinModel;

        mPinTab->update(mWizard->mGateType);

    }

    /*void PinsWizardPage::setGateType(GateType* gate)
    {
        //1
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mPinModel = mWizard->mPinModel;
        mPinTab->update(gate);
    }*/

    void PinsWizardPage::handleDeleteClicked()
    {
        auto treeView = mPinTab->getTreeView();
        auto pinModel = mPinTab->getPinModel();

        pinModel->handleDeleteItem(treeView->currentIndex());
    }

    QList<PinItem*> PinsWizardPage::getPingroups(){
        return mPinModel->getPinGroups();
    }

}

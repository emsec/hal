#include "gui/gatelibrary_management/gatelibrary_pages/pins_wizardpage.h"

#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include <gui/pin_model/pin_delegate.h>

namespace hal
{
    PinsWizardPage::PinsWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Pins and Pingroups");
        setSubTitle("Edit pins and pingroups");
        QGridLayout* layout = new QGridLayout(this);

        mPinTab = new GateLibraryTabPin(this, true);
        mDelBtn = new QPushButton("Delete", this);


        layout->addWidget(mDelBtn, 1, 0);
        layout->addWidget(mPinTab, 0, 0, 1, 2);

        connect(mDelBtn, &QPushButton::clicked, this, &PinsWizardPage::handleDeleteClicked);

    }

    void PinsWizardPage::initializePage()
    {

        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mPinModel = mPinTab->getPinModel();
        mWizard->mPinModel = mPinModel;

        mPinTab->update(mWizard->mGateType);
    }

    void PinsWizardPage::handleDeleteClicked()
    {
        auto treeView = mPinTab->getTreeView();
        auto pinModel = mPinTab->getPinModel();

        pinModel->handleDeleteItem(treeView->currentIndex());
    }

    QList<PinItem*> PinsWizardPage::getPingroups(){
        return mPinModel->getPinGroups();
    }

    bool PinsWizardPage::validatePage()
    {
        for(auto ch : mPinModel->getRootItem()->getChildren()) //check pin direction of groups
        {
            PinItem* pg = static_cast<PinItem*>(ch);
            if(pg->getItemType() != PinItem::TreeItemType::GroupCreator && pg->getDirection() == PinDirection::none) return false;
        }
        return true;
    }
}

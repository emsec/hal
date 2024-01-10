#include "gui/gatelibrary_management/gatelibrary_pages/pins_wizardpage.h"

#include <gui/gatelibrary_management/gatelibrary_wizard.h>
#include <gui/pin_model/pin_delegate.h>

namespace hal
{
    PinsWizardPage::PinsWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 2: Pins and Pingroups");
        setSubTitle("Edit pins and pingroups");
        mLayout = new QGridLayout(this);
        mPinModel = new PinModel(this, true);
        //mPinModel->setGate(new GateType());

        mPinTab = new GateLibraryTabPin(this, true);
        //mAddBtn = new QPushButton("Add", this);
        mDelBtn = new QPushButton("Delete", this);

        mLayout->addWidget(mPinTab, 0, 0, 1, 2);
        mLayout->addWidget(mDelBtn, 1, 0);
        //mLayout->addWidget(mAddBtn, 1, 1);

        connect(mDelBtn, &QPushButton::clicked, this, &PinsWizardPage::handleDeleteClicked);

        setLayout(mLayout);
    }

    void PinsWizardPage::setGateType(GateType* gate)
    {
        mPinTab->update(gate);
    }

    void PinsWizardPage::handleDeleteClicked()
    {
        auto treeView = mPinTab->getTreeView();
        auto pinModel = mPinTab->getPinModel();

        pinModel->handleDeleteItem(treeView->currentIndex());
    }

    int PinsWizardPage::nextId() const
    {
        auto parentWizard = wizard();
        if(!parentWizard)
            return -1;
        return static_cast<GateLibraryWizard*>(parentWizard)->getNextPageId(GateLibraryWizard::Pin);
    }

}

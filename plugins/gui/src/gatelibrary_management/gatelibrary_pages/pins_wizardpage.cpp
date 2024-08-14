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
        mPinModel = mPinTab->getPinModel();

        layout->addWidget(mDelBtn, 1, 0);
        layout->addWidget(mPinTab, 0, 0, 1, 2);

        connect(mDelBtn, &QPushButton::clicked, this, &PinsWizardPage::handleDeleteClicked);
        connect(mPinModel, &PinModel::dataChanged, this, &PinsWizardPage::handlePinModelChanged);
    }

    void PinsWizardPage::initializePage()
    {
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mWizard->mPinModel = mPinModel;

        mPinTab->update(mWizard->mGateType);
    }

    void PinsWizardPage::handleDeleteClicked()
    {
        auto treeView = mPinTab->getTreeView();

        mPinModel->handleDeleteItem(treeView->currentIndex());
        Q_EMIT completeChanged();
    }

    QList<PinItem*> PinsWizardPage::getPingroups(){
        return mPinModel->getPinGroups();
    }

    void PinsWizardPage::handlePinModelChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
    {
        Q_EMIT completeChanged();
    }

    bool PinsWizardPage::isComplete() const
    {
        if(mPinModel->getRootItem()->getChildren().isEmpty()) return false;
        bool hasPingroup = false;
        for(auto ch : mPinModel->getRootItem()->getChildren()) //check pin direction of groups
        {
            PinItem* pg = static_cast<PinItem*>(ch);
            if(pg->getItemType() != PinItem::TreeItemType::GroupCreator) hasPingroup = true;
            if(pg->getItemType() == PinItem::TreeItemType::InvalidPinGroup) return false;
            if(!pg->getChildren().isEmpty())
            {
                for (auto it : pg->getChildren()) {
                    PinItem* p = static_cast<PinItem*>(it);
                    if(pg->getItemType() == PinItem::TreeItemType::InvalidPin) return false;
                }
            }
        }
        return hasPingroup;
    }
}

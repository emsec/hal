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

        mPinTab = new GateLibraryTabPin(this, true);
        mAddBtn = new QPushButton("Add", this);
        mDelBtn = new QPushButton("Del", this);


        mLayout->addWidget(mPinTab, 0, 0, 1, 2);
        mLayout->addWidget(mDelBtn, 1, 0);
        mLayout->addWidget(mAddBtn, 1, 1);

        setLayout(mLayout);
    }

    void PinsWizardPage::setGateType(GateType* gate)
    {
        mPinTab->update(gate);
    }

}

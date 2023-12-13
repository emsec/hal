#include "gui/gatelibrary_management/gatelibrary_pages/pins_wizardpage.h"

namespace hal
{
    PinsWizardPage::PinsWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 2: Pins and Pingroups");
        setSubTitle("Edit pins and pingroups");
        mLayout = new QGridLayout(this);
        //mPinsTreeView = new QTreeView(this);
        mPinTab = new GateLibraryTabPin(this);
        mAddBtn = new QPushButton("Add", this);
        mDelBtn = new QPushButton("Del", this);


        mLayout->addWidget(mPinTab, 0, 0, 1, 2);
        mLayout->addWidget(mDelBtn, 1, 0);
        mLayout->addWidget(mAddBtn, 1, 1);

        setLayout(mLayout);
    }
}

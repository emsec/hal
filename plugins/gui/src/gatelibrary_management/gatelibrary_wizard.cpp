#include "gui/gatelibrary_management/gatelibrary_wizard.h"


#include <QGridLayout>

namespace hal
{
    GateLibraryWizard::GateLibraryWizard(QWidget* parent) : QWizard(parent)
    {
        generalInfoPage = new GeneralInfoWizardPage;
        pinsPage = new PinsWizardPage;
        ffPage = new FlipFlopWizardPage;
        boolPage = new BoolWizardPage;

        generalInfoPage->setTitle("Step 1: General Information");
        pinsPage->setTitle("Step 2: Pins and Pingroups");
        ffPage->setTitle("Step 3: Flip Flop");
        boolPage->setTitle("Step 4: Boolean functions");

        QGridLayout* generalLayout = new QGridLayout;
        QGridLayout* pinsLayout = new QGridLayout;
        QGridLayout* ffLayout = new QGridLayout;
        QGridLayout* boolLayout = new QGridLayout;

        generalInfoPage->setLayout(generalLayout);
        pinsPage->setLayout(pinsLayout);
        ffPage->setLayout(ffLayout);
        boolPage->setLayout(boolLayout);

        this->addPage(generalInfoPage);
        this->addPage(pinsPage);
        this->addPage(ffPage);
        this->addPage(boolPage);
    }
}

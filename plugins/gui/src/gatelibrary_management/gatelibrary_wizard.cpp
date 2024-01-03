#include "gui/gatelibrary_management/gatelibrary_wizard.h"


#include <QGridLayout>

namespace hal
{
    GateLibraryWizard::GateLibraryWizard(const GateLibrary *gateLibrary, GateType *gateType, QWidget* parent): QWizard(parent)
    {
        generalInfoPage = new GeneralInfoWizardPage(gateLibrary, parent);
        pinsPage = new PinsWizardPage(parent);
        ffPage = new FlipFlopWizardPage(parent);
        boolPage = new BoolWizardPage(parent);
        boolPage->setTitle("Step 4: Boolean functions");

        this->addPage(generalInfoPage);
        this->addPage(pinsPage);
        this->addPage(ffPage);
        this->addPage(boolPage);
        mGateLibrary = gateLibrary;
        mGateType = gateType;

        if(mGateType != nullptr)
        {
            QStringList prop = QStringList();
            for (GateTypeProperty p : mGateType->get_property_list()) {
                prop.append(QString::fromStdString(enum_to_string(p)));
            }
            generalInfoPage->setData(QString::fromStdString(mGateType->get_name()), prop);
            pinsPage->setGateType(mGateType);
        }
    }

    void GateLibraryWizard::editGate(GateType* gt)
    {

    }

    void GateLibraryWizard::addGate()
    {

    }

    void GateLibraryWizard::setData(GateLibrary *gateLibrary, GateType* gateType)
    {
        mGateLibrary = gateLibrary;
        mGateType = gateType;
    }
}

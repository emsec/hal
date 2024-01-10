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

        this->addPage(generalInfoPage);
        this->addPage(pinsPage);
        this->addPage(ffPage);
        this->addPage(boolPage);
        mGateLibrary = gateLibrary;
        mGateType = gateType;
        generalInfoPage->setMode(false);

        if(mGateType != nullptr)
        {
            generalInfoPage->setMode(true);
            QStringList prop = QStringList();
            for (GateTypeProperty p : mGateType->get_property_list()) {
                prop.append(QString::fromStdString(enum_to_string(p)));
            }
            generalInfoPage->setData(QString::fromStdString(mGateType->get_name()), prop);
            pinsPage->setGateType(mGateType);
        }
    }

    GateLibraryWizard::GateLibraryWizard(const GateLibrary *gateLibrary, QWidget* parent): QWizard(parent)
    {
        generalInfoPage = new GeneralInfoWizardPage(gateLibrary, parent);
        pinsPage = new PinsWizardPage(parent);
        ffPage = new FlipFlopWizardPage(parent);
        boolPage = new BoolWizardPage(parent);

        this->addPage(generalInfoPage);
        this->addPage(pinsPage);
        this->addPage(ffPage);
        this->addPage(boolPage);
        mGateLibrary = gateLibrary;
        generalInfoPage->setMode(false);
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

    void GateLibraryWizard::accept()
    {
        //TODO: get all the data after user finishes

        mName = generalInfoPage->getName();
        mProperties = generalInfoPage->getProperties();
        this->close();
    }
}

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
        setPageOrder();
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
        setPageOrder();
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

    int GateLibraryWizard::getNextPageId(PAGE page)
    {
        if(!mPageLookupTable.contains(page))
            return -1;

        //TODO logic to check if stateless gate is selected
        if(page == Pin){
            if(mProperties.contains("ff"))
            {
                qInfo() << "skip the next page";
                return mPageLookupTable.value(mPageLookupTable.value(page));
            }
        }
        return mPageLookupTable.value(page);
    }

    void GateLibraryWizard::setPageOrder()
    {
        mPageLookupTable = QMap<PAGE, PAGE>();
        mPageLookupTable.insert(GeneralInfo, Pin);
        mPageLookupTable.insert(Pin, FlipFlop);
        mPageLookupTable.insert(FlipFlop, BooleanFunction);
        mPageLookupTable.insert(BooleanFunction, None);
        mPageLookupTable.insert(None, None);
    }
}

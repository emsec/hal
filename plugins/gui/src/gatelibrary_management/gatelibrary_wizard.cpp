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

        latchPage = new LatchWizardPage(parent);
        lutPage = new LUTWizardPage(parent);
        initPage = new InitWizardPage(parent);
        ramPage = new RAMWizardPage(parent);

        /*this->addPage(generalInfoPage);
        this->addPage(pinsPage);
        this->addPage(ffPage);

        this->addPage(latchPage);
        this->addPage(lutPage);
        this->addPage(ramPage);
        this->addPage(initPage);

        this->addPage(boolPage);*/

        setPage(GeneralInfo, generalInfoPage);
        setPage(Pin, pinsPage);
        setPage(FlipFlop, ffPage);
        setPage(Latch, latchPage);
        setPage(LUT, lutPage);
        setPage(RAM, ramPage);
        setPage(Init, initPage);
        setPage(BooleanFunction, boolPage);

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
            ffPage->setData(mGateType);
            latchPage->setData(mGateType);
            lutPage->setData(mGateType);
            initPage->setData(mGateType);
            ramPage->setData(mGateType);
            pinsPage->setGateType(mGateType);
        }
        //setPageOrder();
    }

    GateLibraryWizard::GateLibraryWizard(const GateLibrary *gateLibrary, QWidget* parent): QWizard(parent)
    {
        generalInfoPage = new GeneralInfoWizardPage(gateLibrary, parent);
        pinsPage = new PinsWizardPage(parent);
        ffPage = new FlipFlopWizardPage(parent);
        latchPage = new LatchWizardPage(parent);
        lutPage = new LUTWizardPage(parent);
        initPage = new InitWizardPage(parent);
        ramPage = new RAMWizardPage(parent);
        boolPage = new BoolWizardPage(parent);

        /*this->addPage(generalInfoPage);
        this->addPage(pinsPage);
        this->addPage(ffPage);
        this->addPage(latchPage);
        this->addPage(lutPage);
        this->addPage(ramPage);
        this->addPage(initPage);
        this->addPage(boolPage);*/
        setPage(GeneralInfo, generalInfoPage);
        setPage(Pin, pinsPage);
        setPage(FlipFlop, ffPage);
        setPage(Latch, latchPage);
        setPage(LUT, lutPage);
        setPage(RAM, ramPage);
        setPage(Init, initPage);
        setPage(BooleanFunction, boolPage);
        pinsPage->setGateType(nullptr);
        mGateLibrary = gateLibrary;
        generalInfoPage->setMode(false);
        ffPage->setData(nullptr);
        //setPageOrder();
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
        mPingroups = pinsPage->getPingroups();

        this->close();
    }

    QStringList GateLibraryWizard::getProperties()
    {
        return mProperties;
    }

    int GateLibraryWizard::nextId() const
    {
        const QStringList properties = generalInfoPage->getProperties();

        switch(currentId()){
        case GeneralInfo:
            return Pin;
        case Pin:
            if(properties.contains("ff")) return FlipFlop;
            else if(properties.contains("latch")) return Latch;
            else if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;
            else return Init;
        case FlipFlop:
            if(properties.contains("latch")) return Latch;
            else if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;
            else return Init;
        case Latch:
            if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;
            else return Init;
        case LUT:
            if(properties.contains("ram")) return RAM;
            else return Init;
        case RAM:
            return Init;
        case Init:
            return BooleanFunction;
        case BooleanFunction:
        default:
            return -1;
        }
    }
}

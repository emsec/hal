#include "gui/gatelibrary_management/gatelibrary_wizard.h"


#include <QGridLayout>

namespace hal
{
    GateLibraryWizard::GateLibraryWizard(const GateLibrary *gateLibrary, GateType *gateType, QWidget* parent): QWizard(parent)
    {
        generalInfoPage = new GeneralInfoWizardPage(gateLibrary, this);
        pinsPage = new PinsWizardPage(this);
        ffPage = new FlipFlopWizardPage(this);
        boolPage = new BoolWizardPage(this);

        latchPage = new LatchWizardPage(this);
        lutPage = new LUTWizardPage(this);
        initPage = new InitWizardPage(this);
        ramPage = new RAMWizardPage(this);
        ramportPage = new RAMPortWizardPage(this);
        statePage = new StateWizardPage(this);

        setPage(GeneralInfo, generalInfoPage);
        setPage(Pin, pinsPage);
        setPage(FlipFlop, ffPage);
        setPage(Latch, latchPage);
        setPage(LUT, lutPage);
        setPage(RAM, ramPage);
        setPage(RAMPort, ramportPage);
        setPage(Init, initPage);
        setPage(State, statePage);
        setPage(BooleanFunction, boolPage);

        mGateLibrary = gateLibrary;
        mGateType = gateType;
        generalInfoPage->setMode(false);
        mPinModel = new PinModel(this, true);
        mPinTab = new GateLibraryTabPin(this, true);

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
            ramportPage->setData(mGateType);
            statePage->setData(mGateType);
            //pinsPage->setGateType(mGateType);
        }
    }

    GateLibraryWizard::GateLibraryWizard(const GateLibrary *gateLibrary, QWidget* parent): QWizard(parent)
    {
        generalInfoPage = new GeneralInfoWizardPage(gateLibrary, this);
        pinsPage = new PinsWizardPage(this);
        ffPage = new FlipFlopWizardPage(this);
        boolPage = new BoolWizardPage(this);

        latchPage = new LatchWizardPage(this);
        lutPage = new LUTWizardPage(this);
        initPage = new InitWizardPage(this);
        ramPage = new RAMWizardPage(this);
        ramportPage = new RAMPortWizardPage(this);
        statePage = new StateWizardPage(this);

        setPage(GeneralInfo, generalInfoPage);
        setPage(Pin, pinsPage);
        setPage(FlipFlop, ffPage);
        setPage(Latch, latchPage);
        setPage(LUT, lutPage);
        setPage(RAM, ramPage);
        setPage(RAMPort, ramportPage);
        setPage(Init, initPage);
        setPage(State, statePage);
        setPage(BooleanFunction, boolPage);
        //pinsPage->setGateType(nullptr);
        generalInfoPage->setMode(false);
        ffPage->setData(nullptr);
        mGateLibrary = gateLibrary;

        mPinTab = new GateLibraryTabPin(this, true);
        mPinModel = new PinModel(this, true);
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
        //mPingroups = pinsPage->getPingroups();

        this->close();
    }

    QStringList GateLibraryWizard::getProperties()
    {
        return mProperties;
    }

    QList<PinItem*> GateLibraryWizard::getPingroups()
    {
        return mPinModel->getPinGroups();
    }

    int GateLibraryWizard::nextId() const
    {
        const QStringList properties = generalInfoPage->getProperties();

        switch(currentId()){
        case GeneralInfo:
            return Pin;
        case Pin:
            /*if(properties.contains("ff")) return FlipFlop;
            else if(properties.contains("latch")) return Latch;
            else if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;*/
            return BooleanFunction;
        case BooleanFunction:
            if(properties.contains("ff")) return FlipFlop;
            else if(properties.contains("latch")) return Latch;
            else if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;
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
            return RAMPort; //TODO
        case RAMPort:
            if(properties.contains("ff") || properties.contains("latch")) return State;
            else return Init;
        case State:
            if(properties.contains("ff") || properties.contains("latch") || properties.contains("c_lut") || properties.contains("ram")) return Init;
            else return -1;
        case Init:
        default:
            return -1;
        }
    }
}

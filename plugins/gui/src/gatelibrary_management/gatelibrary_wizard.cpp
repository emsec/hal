#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/netlist/boolean_function.h"


#include <QGridLayout>

namespace hal
{
    GateLibraryWizard::GateLibraryWizard(GateLibrary *gateLibrary, GateType *gateType, QWidget* parent): QWizard(parent)
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
        setPage(BoolFunc, boolPage);

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
            boolPage->setData(mGateType);
            //pinsPage->setGateType(mGateType);
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

    void GateLibraryWizard::accept()
    {
        //TODO: get all the data after user finishes

        mName = generalInfoPage->getName();
        mProperties = generalInfoPage->getProperties();
        //mPingroups = pinsPage->getPingroups();
        std::set<GateTypeProperty> properties_set;
        for(QString prop : generalInfoPage->getProperties())
        {
            //Convert QStringList to std::set
            properties_set.insert(enum_from_string<GateTypeProperty>(prop.toStdString()));

        }
        //Set name and properties
        GateType* newGateType = mGateLibrary->create_gate_type(generalInfoPage->getName().toStdString(), properties_set);

        //Set pingroups and pins
        for(PinItem* pingroup : getPingroups())
        {
            std::vector<GatePin*> gatepins;
            for (auto it : pingroup->getChildren()) {
                PinItem* pin = static_cast<PinItem*>(it);
                gatepins.push_back(new GatePin(pin->getId(), pin->getName().toStdString(), pin->getDirection(), pin->getPinType()));
            }
            newGateType->create_pin_group(pingroup->getName().toStdString(), gatepins, pingroup->getDirection(), pingroup->getPinType());
        }


        //Set boolean functions
        newGateType->add_boolean_functions(boolPage->getBoolFunctions());

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

    std::unique_ptr<GateTypeComponent> GateLibraryWizard::getComponents()
    {
        for(QString prop : generalInfoPage->getProperties())
        {
            //Set components
            switch(enum_from_string<GateTypeComponent::ComponentType>(prop.toStdString())){
            case GateTypeComponent::ComponentType::lut:
            {
                std::unique_ptr<GateTypeComponent> ff_comp = GateTypeComponent::create_ff_component()
                break;
            }
            case GateTypeComponent::ComponentType::ff:
            {

            }
            case GateTypeComponent::ComponentType::latch:
            {
                std::unique_ptr<GateTypeComponent> latch_comp = GateTypeComponent::create_latch_component()
                break;
            }
            case GateTypeComponent::ComponentType::ram:
            {
                std::unique_ptr<GateTypeComponent> ram_comp = GateTypeComponent::create_ram_component();
                break;
            }
            case GateTypeComponent::ComponentType::mac:
            {
                std::unique_ptr<GateTypeComponent> mac_comp = GateTypeComponent::create_mac_component();
                break;
            }
            case GateTypeComponent::ComponentType::init:
            {
                std::string category = initPage->mCategory->text().toStdString();
                QStringList ids = initPage->mIdentifiers->toPlainText().split('\n', Qt::SkipEmptyParts);
                std::vector<std::string> identifiers;
                for(QString id : ids) identifiers.push_back(id.toStdString());
                std::unique_ptr<GateTypeComponent> init_comp = GateTypeComponent::create_init_component(category, identifiers);
                break;
            }
            case GateTypeComponent::ComponentType::state:
            {
                std::unique_ptr<GateTypeComponent> state_comp = GateTypeComponent::create_state_component()
                break;
            }
            case GateTypeComponent::ComponentType::ram_port:
            {
                std::unique_ptr<GateTypeComponent> ram_port_comp = GateTypeComponent::create_ram_port_component()
                break;
            }
            }
        }

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
        case FlipFlop:
            if(properties.contains("latch")) return Latch;
            else if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;
            else return BoolFunc;
        case Latch:
            if(properties.contains("c_lut")) return LUT;
            else if(properties.contains("ram")) return RAM;
            else return BoolFunc;
        case LUT:
            if(properties.contains("ram")) return RAM;
            else return BoolFunc;
        case RAM:
            return RAMPort;
        case RAMPort:
            if(properties.contains("ff") || properties.contains("latch")) return State;
            else return BoolFunc;
        case State:
            if(properties.contains("ff") || properties.contains("latch") || properties.contains("c_lut") || properties.contains("ram")) return Init;
            else return BoolFunc;
        case Init:
            return BoolFunc;
        case BoolFunc:
        default:
            return -1;
        }
    }
}

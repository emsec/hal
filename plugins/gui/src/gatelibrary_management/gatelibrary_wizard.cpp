#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"


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

    GateType* GateLibraryWizard::addGate()
    {
        std::set<GateTypeProperty> properties_set;
        for(QString prop : generalInfoPage->getProperties())
        {
            //Convert QStringList to std::set
            properties_set.insert(enum_from_string<GateTypeProperty>(prop.toStdString()));

        }
        //Set name, properties and the parent component
        mNewGateType = mGateLibrary->create_gate_type(generalInfoPage->getName().toStdString(), properties_set, setComponents());
        //Set pingroups and pins
        for(PinItem* pingroup : getPingroups())
        {
            std::vector<GatePin*> gatepins;
            for (auto it : pingroup->getChildren()) {
                PinItem* pin = static_cast<PinItem*>(it);
                if(pin->getItemType() != PinItem::TreeItemType::PinCreator)
                {
                    auto res = mNewGateType->create_pin(pin->getName().toStdString(), pin->getDirection(), pin->getPinType());
                    if(res.is_ok()) gatepins.push_back(res.get());
                }
            }
            if(pingroup->getItemType() != PinItem::TreeItemType::GroupCreator)
                mNewGateType->create_pin_group(pingroup->getName().toStdString(), gatepins, pingroup->getDirection(), pingroup->getPinType());
        }

        //Set boolean functions
        mNewGateType->add_boolean_functions(boolPage->getBoolFunctions());
        return mNewGateType;
    }

    void GateLibraryWizard::setData(GateLibrary *gateLibrary, GateType* gateType)
    {
        mGateLibrary = gateLibrary;
        mGateType = gateType;
    }

    void GateLibraryWizard::accept()
    {
        //TODO: get all the data after user finishes

        if(!mGateLibrary->contains_gate_type_by_name(generalInfoPage->getName().toStdString())) addGate();
        else editGate(mGateLibrary->get_gate_type_by_name(generalInfoPage->getName().toStdString()));

        this->close();
    }

    GateType* GateLibraryWizard::getRecentCreatedGate(){
        return mNewGateType;
    }

    QList<PinItem*> GateLibraryWizard::getPingroups()
    {
        return mPinModel->getPinGroups();
    }

    std::unique_ptr<GateTypeComponent> GateLibraryWizard::setComponents()
    {
        std::unique_ptr<GateTypeComponent> parentComponent;
        for(QString prop : generalInfoPage->getProperties())
        {
            //Set components
            if(prop == "c_lut")
            {
                std::unique_ptr<GateTypeComponent> init_comp(mGateType->get_component([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }));
                if(init_comp == nullptr) //must be used with init -> create init component
                {
                    std::string category = initPage->mCategory->text().toStdString();
                    QStringList ids = initPage->mIdentifiers->toPlainText().split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts);
                    std::vector<std::string> identifiers;
                    for(QString id : ids) identifiers.push_back(id.toStdString());
                    init_comp = parentComponent->create_init_component(category, identifiers);

                }
                //create lut component
                parentComponent->create_lut_component(std::move(init_comp), lutPage->mAscending->text()=="Ascending");
                break;
            }
            if(prop == "ff")
            {
                //must be used with state -> create state component
                //can be used with init -> create init if not empty
                //create ff component


                //std::unique_ptr<GateTypeComponent> ff_comp = GateTypeComponent::create_ff_component()
            }
            if(prop == "latch")
            {
                //must be used with state -> create state component
                //create latch component
                //std::unique_ptr<GateTypeComponent> latch_comp = GateTypeComponent::create_latch_component()
                break;
            }
            if(prop == "ram")
            {
                //must be used with at least one ram_port -> create ram_port component
                //can be used with init -> create init if not empty
                //create ram component

                //std::unique_ptr<GateTypeComponent> ram_comp = GateTypeComponent::create_ram_component();
                break;
            }
            /*if(prop == "mac")
            {
                std::unique_ptr<GateTypeComponent> mac_comp = GateTypeComponent::create_mac_component();
                break;
            }*/

            //INIT is NOT in enum GateTypeProperty!
            /*if(prop == "init")
            {
                std::string category = initPage->mCategory->text().toStdString();
                QStringList ids = initPage->mIdentifiers->toPlainText().split('\n', Qt::SkipEmptyParts);
                std::vector<std::string> identifiers;
                for(QString id : ids) identifiers.push_back(id.toStdString());
                std::unique_ptr<GateTypeComponent> init_comp = GateTypeComponent::create_init_component(category, identifiers);
                return init_comp;
                break;
            }*/

            //STATE is NOT in enum GateTypeProperty!
            /*if(prop == "state")
            {
                //std::unique_ptr<GateTypeComponent> state_comp = GateTypeComponent::create_state_component()
                break;
            }*/

            //RAM_PORT is NOT in enum GateTypeProperty!
            /*if(prop == "ram_port")
            {
                //std::unique_ptr<GateTypeComponent> ram_port_comp = GateTypeComponent::create_ram_port_component()
                break;
            }*/
        }
        return parentComponent;
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

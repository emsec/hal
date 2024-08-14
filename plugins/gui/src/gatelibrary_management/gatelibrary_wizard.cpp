#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"


#include <QGridLayout>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

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
        mPinModel = new PinModel(this, true);
        mPinTab = new GateLibraryTabPin(this, true);

        if(mGateType == nullptr)
        {
            setWindowTitle("Create new gate type");
        }
        else
        {
            setWindowTitle("Modify gate type " + QString::fromStdString(mGateType->get_name()));

            generalInfoPage->setData(QString::fromStdString(mGateType->get_name()), mGateType->get_property_list());
            ffPage->setData(mGateType);
            latchPage->setData(mGateType);
            lutPage->setData(mGateType);
            initPage->setData(mGateType);
            ramPage->setData(mGateType);
            ramportPage->setData(mGateType);
            statePage->setData(mGateType);
            boolPage->setData(mGateType);
        }
    }

    void GateLibraryWizard::accept()
    {
        //Convert QStringList to std::set
        std::set<GateTypeProperty> properties_set;
        for(GateTypeProperty prop : generalInfoPage->getProperties())
            properties_set.insert(prop);

        if(mGateType)
        {
            mGateType = mGateLibrary->replace_gate_type(mGateType->get_id(), generalInfoPage->getName().toStdString(), properties_set, setComponents());
            //Set pingroups and pins
            for(PinItem* pingroup : getPingroups())
            {
                std::vector<GatePin*> gatepins;
                for (auto it : pingroup->getChildren()) {
                    PinItem* pin = static_cast<PinItem*>(it);
                    if(pin->getItemType() != PinItem::TreeItemType::PinCreator)
                    {
                        auto res = mGateType->create_pin(pin->getName().toStdString(), pin->getDirection(), pin->getPinType());
                        if(res.is_ok()) gatepins.push_back(res.get());
                    }
                }
                if(pingroup->getItemType() != PinItem::TreeItemType::GroupCreator)
                    mGateType->create_pin_group(pingroup->getName().toStdString(), gatepins, pingroup->getDirection(), pingroup->getPinType());
            }

            //TODO: Set boolean functions without adding the same double
            //also: boolean functions may be removed
            mGateType->add_boolean_functions(boolPage->getBoolFunctions());
        }
        else
        {
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
        }


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
        for(GateTypeProperty prop : generalInfoPage->getProperties())
        {
            //Set components
            if(prop == GateTypeProperty::c_lut)
            {
                std::vector<std::string> identifiers;
                for (QString id : initPage->mIdentifiers->toPlainText().split('\n', SKIP_EMPTY_PARTS) ) {
                    identifiers.push_back(id.toStdString());
                }
                std::unique_ptr<GateTypeComponent> init_component = GateTypeComponent::create_init_component(initPage->mCategory->text().toStdString(), identifiers);
                parentComponent = GateTypeComponent::create_lut_component(std::move(init_component), lutPage->mAscending->isChecked());
            }
            else if(prop == GateTypeProperty::ff)
            {
                std::vector<std::string> identifiers;
                for (QString id : initPage->mIdentifiers->toPlainText().split('\n', SKIP_EMPTY_PARTS) ) {
                    identifiers.push_back(id.toStdString());
                }
                std::unique_ptr<GateTypeComponent> init_component = GateTypeComponent::create_init_component(initPage->mCategory->text().toStdString(), identifiers);
                std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(std::move(init_component),
                                                                                                               statePage->mStateIdentifier->text().toStdString(),
                                                                                                               statePage->mNegStateIdentifier->text().toStdString());
                BooleanFunction next_state_bf;
                BooleanFunction clock_bf;

                auto next_state_res = BooleanFunction::from_string(ffPage->mNextState->text().toStdString());
                auto clock_bf_res = BooleanFunction::from_string(ffPage->mClock->text().toStdString());

                if(next_state_res.is_ok()) next_state_bf = next_state_res.get();
                if(clock_bf_res.is_ok()) clock_bf = clock_bf_res.get();

                std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_ff_component(std::move(state_component),
                                                                                                      next_state_bf,
                                                                                                      clock_bf);
                FFComponent* ff_component = component->convert_to<FFComponent>();

                BooleanFunction async_reset;
                auto async_reset_res = BooleanFunction::from_string(ffPage->mAReset->text().toStdString());
                if(async_reset_res.is_ok()) async_reset = async_reset_res.get();
                ff_component->set_async_reset_function(async_reset);

                BooleanFunction async_set;
                auto async_set_res = BooleanFunction::from_string(ffPage->mASet->text().toStdString());
                if(async_set_res.is_ok()) async_set = async_set_res.get();
                ff_component->set_async_set_function(async_set);

                const auto behav_state = enum_from_string<AsyncSetResetBehavior>(ffPage->mIntState->text().toStdString(), AsyncSetResetBehavior::undef);
                const auto behav_neg_state = enum_from_string<AsyncSetResetBehavior>(ffPage->mNegIntState->text().toStdString(), AsyncSetResetBehavior::undef);
                ff_component->set_async_set_reset_behavior(behav_state, behav_neg_state);
                parentComponent = std::move(component);
            }
            else if(prop == GateTypeProperty::latch)
            {
                std::unique_ptr<GateTypeComponent> state_component = GateTypeComponent::create_state_component(nullptr,
                                                                                                               statePage->mStateIdentifier->text().toStdString(),
                                                                                                               statePage->mNegStateIdentifier->text().toStdString());
                std::unique_ptr<GateTypeComponent> component = GateTypeComponent::create_latch_component(std::move(state_component));
                LatchComponent* latch_component = component->convert_to<LatchComponent>();

                auto data_in_res = BooleanFunction::from_string(latchPage->mDataIn->text().toStdString());
                auto enable_res = BooleanFunction::from_string(latchPage->mEnableOn->text().toStdString());

                if(data_in_res.is_ok()) latch_component->set_data_in_function(data_in_res.get());
                if(enable_res.is_ok()) latch_component->set_enable_function(enable_res.get());

                BooleanFunction async_reset;
                auto async_reset_res = BooleanFunction::from_string(latchPage->mAReset->text().toStdString());
                if(async_reset_res.is_ok()) async_reset = async_reset_res.get();
                latch_component->set_async_reset_function(async_reset);

                BooleanFunction async_set;
                auto async_set_res = BooleanFunction::from_string(latchPage->mASet->text().toStdString());
                if(async_set_res.is_ok()) async_set = async_set_res.get();
                latch_component->set_async_set_function(async_set);

                const auto behav_state = enum_from_string<AsyncSetResetBehavior>(latchPage->mIntState->text().toStdString(), AsyncSetResetBehavior::undef);
                const auto behav_neg_state = enum_from_string<AsyncSetResetBehavior>(latchPage->mNegIntState->text().toStdString(), AsyncSetResetBehavior::undef);
                latch_component->set_async_set_reset_behavior(behav_state, behav_neg_state);

                parentComponent = std::move(component);
            }
            else if(prop == GateTypeProperty::ram)
            {
                std::unique_ptr<GateTypeComponent> sub_component = nullptr;
                std::vector<std::string> identifiers;
                for (QString id : initPage->mIdentifiers->toPlainText().split('\n', SKIP_EMPTY_PARTS) )
                    identifiers.push_back(id.toStdString());
                sub_component = GateTypeComponent::create_init_component(initPage->mCategory->text().toStdString(), identifiers);

                for (RAMPortWizardPage::RAMPort rpEdit : ramportPage->getRamPorts()) {
                    BooleanFunction clocked_on_bf;
                    auto clocked_on_res = BooleanFunction::from_string(rpEdit.clockFunction->text().toStdString());
                    if(clocked_on_res.is_ok()) clocked_on_bf = clocked_on_res.get();

                    BooleanFunction enabled_on_bf;
                    auto enabled_on_res = BooleanFunction::from_string(rpEdit.enableFunciton->text().toStdString());
                    if(enabled_on_res.is_ok()) enabled_on_bf = enabled_on_res.get();

                    sub_component = GateTypeComponent::create_ram_port_component(
                                std::move(sub_component),
                                rpEdit.dataGroup->text().toStdString(),
                                rpEdit.addressGroup->text().toStdString(),
                                clocked_on_bf,
                                enabled_on_bf,
                                rpEdit.isWritePort->isChecked());
                }
                parentComponent = GateTypeComponent::create_ram_component(std::move(sub_component), ramPage->mBitSize->text().toInt());
            }
        }
        return std::move(parentComponent);
    }

    int GateLibraryWizard::nextId() const
    {
        const QList<GateTypeProperty> properties = generalInfoPage->getProperties();

        switch(currentId()){
        case GeneralInfo:
            return Pin;
        case Pin:
            if(properties.contains(GateTypeProperty::ff) || properties.contains(GateTypeProperty::latch)) return State;
            /*if(properties.contains(GateTypeProperty::ff)) return FlipFlop;
            else if(properties.contains(GateTypeProperty::latch)) return Latch;*/
            else if(properties.contains(GateTypeProperty::c_lut)) return LUT;
            else if(properties.contains(GateTypeProperty::ram)) return RAM;
            return BoolFunc;  
        case State:
            if(properties.contains(GateTypeProperty::ff)) return FlipFlop;
            else if (properties.contains(GateTypeProperty::latch)) return Latch;
            else if (properties.contains(GateTypeProperty::c_lut)) return LUT;
            else if (properties.contains(GateTypeProperty::ram)) return Init;
            return BoolFunc;
        case FlipFlop:
            if(properties.contains(GateTypeProperty::latch)) return Latch;
            else if(properties.contains(GateTypeProperty::c_lut)) return LUT;
            else if(properties.contains(GateTypeProperty::ram)) return RAM;
            return BoolFunc;
        case Latch:
            if(properties.contains(GateTypeProperty::c_lut)) return LUT;
            else if(properties.contains(GateTypeProperty::ram)) return RAM;
            return BoolFunc;
        case LUT:
            if(properties.contains(GateTypeProperty::ram)) return RAM;
            return Init;
        case RAM:
            return RAMPort;
        case RAMPort:
            if(properties.contains(GateTypeProperty::ff) || properties.contains(GateTypeProperty::latch)) return State;
            return Init;
        case Init:
            if(properties.contains(GateTypeProperty::c_lut)) return -1;
            return BoolFunc;
        case BoolFunc:
        default:
            return -1;
        }
    }
}

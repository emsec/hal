#include "gui/python/py_code_provider.h"

//------
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "gui/gui_globals.h"

namespace hal
{
    const QString PyCodeProvider::gateCodePrefix = "netlist.get_gate_by_id(%1)";
    const QString PyCodeProvider::netCodePrefix = "netlist.get_net_by_id(%1)";
    const QString PyCodeProvider::moduleCodePrefix = "netlist.get_module_by_id(%1)";
    const QString PyCodeProvider::groupingCodePrefix = "netlist.get_grouping_by_id(%1)";

    QString PyCodeProvider::buildPyCode(const QString& prefix, const QString& suffix, u32 id)
    {
        return QString(prefix + "." + suffix).arg(id);
    }

    QString PyCodeProvider::pyCodeGate(u32 gateId)
    {
        return gateCodePrefix.arg(gateId);
    }

    QString PyCodeProvider::pyCodeGateId(u32 gateId)
    {
        const QString suffix = "get_id()";

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateName(u32 gateId)
    {
        const QString suffix = "get_name()";

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateType(u32 gateId)
    {
        const QString suffix = "get_type()";

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateTypePinDirection(u32 gateID, QString pin)
    {
        return pyCodeGateType(gateID) + QString(".get_pin_direction(\"%1\")").arg(pin);
    }

    QString PyCodeProvider::pyCodeGateTypePinType(u32 gateID, QString pin)
    {
        return pyCodeGateType(gateID) + QString(".get_pin_type(\"%1\")").arg(pin);
    }

    QString PyCodeProvider::pyCodeGateBooleanFunction(u32 gateId, QString booleanFunctionName)
    {
        const QString suffix = QString("get_boolean_function(\"%1\")").arg(booleanFunctionName);

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateAsyncSetResetBehavior(u32 gateId)
    {   
        //logic (ff/latch check) can also be done beforehand to remove any logic from
        //this class, but could "blow up" the code if many other places need to call this function

        GateType* gt = gNetlist->get_gate_by_id(gateId)->get_type();
        QString pyString = pyCodeGateType(gateId) + ".get_component(filter = lambda f: hal_py.%1.is_class_of(f)).get_async_set_reset_behavior()";

        if(FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); }); ff_component != nullptr)
            pyString = pyString.arg("FFComponent");
        else if(LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c) { return LatchComponent::is_class_of(c); }); latch_component != nullptr)
            pyString = pyString.arg("LatchComponent");
        else
            pyString = "";

        return pyString;
    }

    QString PyCodeProvider::pyCodeGateDataMap(u32 gateId)
    {
        const QString suffix = QString("get_data_map()");
        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeStateComp(u32 gateId)
    {
        return pyCodeGateType(gateId) + ".get_component(filter = hal_py.StateComponent.is_class_of)";
    }

    QString PyCodeProvider::pyCodeStateCompPosState(u32 gateId)
    {
        return pyCodeStateComp(gateId) + ".get_state_identifier()";
    }

    QString PyCodeProvider::pyCodeStateCompNegState(u32 gateId)
    {
        return pyCodeStateComp(gateId) + ".get_neg_state_identifier()";
    }

    QString PyCodeProvider::pyCodeFFComp(u32 gateId)
    {
        return pyCodeGateType(gateId) + ".get_component(filter = hal_py.FFComponent.is_class_of)";
    }

    QString PyCodeProvider::pyCodeFFCompClockFunc(u32 gateId)
    {
        return pyCodeFFComp(gateId) + ".get_clock_function()";
    }

    QString PyCodeProvider::pyCodeFFCompNextStateFunc(u32 gateId)
    {
        return pyCodeFFComp(gateId) + ".get_next_state_function()";
    }

    QString PyCodeProvider::pyCodeFFCompAsyncSetFunc(u32 gateId)
    {
        return pyCodeFFComp(gateId) + ".get_async_set_function()";
    }

    QString PyCodeProvider::pyCodeFFCompAsyncResetFunc(u32 gateId)
    {
        return pyCodeFFComp(gateId) + ".get_async_reset_function()";
    }

    QString PyCodeProvider::pyCodeFFCompSetResetBehav(u32 gateId)
    {
        return pyCodeFFComp(gateId) + ".get_async_set_reset_behavior()";
    }

    QString PyCodeProvider::pyCodeLatchComp(u32 gateId)
    {
        return pyCodeGateType(gateId) + ".get_component(filter = hal_py.LatchComponent.is_class_of)";
    }

    QString PyCodeProvider::pyCodeLatchCompEnableFunc(u32 gateId)
    {
        return pyCodeLatchComp(gateId) + ".get_enable_function()";
    }

    QString PyCodeProvider::pyCodeLatchCompDataInFunc(u32 gateId)
    {
        return pyCodeLatchComp(gateId) + ".get_data_in_function()";
    }

    QString PyCodeProvider::pyCodeLatchCompAsyncSetFunc(u32 gateId)
    {
        return pyCodeLatchComp(gateId) + ".get_async_set_function()";
    }

    QString PyCodeProvider::pyCodeLatchCompAsyncResetFunc(u32 gateId)
    {
        return pyCodeLatchComp(gateId) + ".get_async_reset_function()";
    }

    QString PyCodeProvider::pyCodeLatchCompSetResetBehav(u32 gateId)
    {
        return pyCodeLatchComp(gateId) + ".get_async_set_reset_behavior()";
    }

    QString PyCodeProvider::pyCodeProperties(u32 gateId)
    {
        const QString suffix = "get_type().get_properties()";

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateLocation(u32 gateId)
    {
        const QString suffix = "get_location()";

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateModule(u32 gateId)
    {
        const QString suffix = "get_module()";

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeGateData(u32 gateId, QString category, QString key)
    {
        const QString suffix = QString("data[(\"%1\", \"%2\")]").arg(category, key);

        return buildPyCode(gateCodePrefix, suffix, gateId);
    }

    QString PyCodeProvider::pyCodeNet(u32 netId)
    {
        return netCodePrefix.arg(netId);
    }

    QString PyCodeProvider::pyCodeNetId(u32 netId)
    {
        const QString suffix = "get_id()";

        return buildPyCode(netCodePrefix, suffix, netId);
    }

    QString PyCodeProvider::pyCodeNetName(u32 netId)
    {
        const QString suffix = "get_name()";

        return buildPyCode(netCodePrefix, suffix, netId);
    }

    QString PyCodeProvider::pyCodeNetType(u32 netId)
    {
        const QString prefix = QString(netCodePrefix).arg(netId);

        const QString check1 = prefix + ".is_global_input_net()";
        const QString check2 = prefix + ".is_global_output_net()";
        const QString check3 = prefix + ".is_unrouted()";

        const QString result1 = "\"Global Input\"";
        const QString result2 = "\"Global Output\"";
        const QString result3 = "\"Unrouted\"";
        const QString result4 = "\"Internal\"";

        const QString pyCode = result1 + " if " + check1 + " else " + result2 + " if " + check2 + " else " + result3 + " if " + check3 + " else " + result4;

        return pyCode;
    }

    QString PyCodeProvider::pyCodeNetData(u32 netId, QString category, QString key)
    {
        const QString suffix = QString("data[(\"%1\", \"%2\")]").arg(category, key);

        return buildPyCode(netCodePrefix, suffix, netId);
    }

    QString PyCodeProvider::pyCodeNetDataMap(u32 netId)
    {
        const QString suffix = QString("get_data_map()");
        return buildPyCode(netCodePrefix, suffix, netId);
    }

    QString PyCodeProvider::pyCodeModule(u32 moduleId)
    {
        return moduleCodePrefix.arg(moduleId);
    }

    QString PyCodeProvider::pyCodeModuleId(u32 moduleId)
    {
        const QString suffix = "get_id()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleName(u32 moduleId)
    {
        const QString suffix = "get_name()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleType(u32 moduleId)
    {
        const QString suffix = "get_type()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);

    }

    QString PyCodeProvider::pyCodeModuleModule(u32 moduleId)
    {
        const QString suffix = "get_parent_module()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleSubmodules(u32 moduleId)
    {
        const QString suffix = "get_submodules()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleGates(u32 moduleId, bool recursively)
    {
        QString rec = recursively ? QString("None, True") : QString("");
        const QString suffix = "get_gates(%2)";
        return QString(moduleCodePrefix + "." + suffix).arg(QString::number(moduleId), rec);
    }

    QString PyCodeProvider::pyCodeModuleNets(u32 moduleId)
    {
        const QString suffix = "get_nets()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleInputNets(u32 moduleId)
    {
        const QString suffix = "get_input_nets()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleOutputNets(u32 moduleId)
    {
        const QString suffix = "get_output_nets()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleInternalNets(u32 moduleId)
    {
        const QString suffix = "get_internal_nets()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleIsTopModule(u32 moduleId)
    {
        const QString suffix = "is_top_module()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleData(u32 moduleId, QString category, QString key)
    {
        const QString suffix = QString("data[(\"%1\", \"%2\")]").arg(category, key);

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModuleDataMap(u32 moduleId)
    {
        const QString suffix = QString("get_data_map()");
        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModulePinGroup(u32 moduleId, QString groupName)
    {
        const QString suffix = QString("get_pin_group(\"%1\")").arg(groupName);

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModulePinGroups(u32 moduleId)
    {
        const QString suffix = "get_pin_groups()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModulePinGroupName(u32 moduleId, QString groupName)
    {
        return pyCodeModulePinGroup(moduleId, groupName) + ".get_name()";
    }

    QString PyCodeProvider::pyCodeModulePinByName(u32 moduleId, QString pinName)
    {
        const QString suffix = QString("get_pin(\"%1\")").arg(pinName);

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeModulePinName(u32 moduleId, QString pinName)
    {
        return pyCodeModulePinByName(moduleId, pinName) + ".get_name()";
    }

    QString PyCodeProvider::pyCodeModulePinDirection(u32 moduleId, QString pinName)
    {
        return pyCodeModulePinByName(moduleId, pinName) + ".get_direction()";
    }

    QString PyCodeProvider::pyCodeModulePinType(u32 moduleId, QString pinName)
    {
        return pyCodeModulePinByName(moduleId, pinName) + ".get_type()";
    }

    QString PyCodeProvider::pyCodeModulePins(u32 moduleId)
    {
        static QString suffix = "get_pins()";

        return buildPyCode(moduleCodePrefix, suffix, moduleId);
    }

    QString PyCodeProvider::pyCodeGrouping(u32 groupingId)
    {
        return groupingCodePrefix.arg(groupingId);
    }

    QString PyCodeProvider::pyCodeGroupingName(u32 groupingId)
    {
        const QString suffix = QString("get_name()");

        return buildPyCode(groupingCodePrefix, suffix, groupingId);
    }

    QString PyCodeProvider::pyCodeGroupingId(u32 groupingId)
    {
        const QString suffix = QString("get_id()");
        
        return buildPyCode(groupingCodePrefix, suffix, groupingId);
    }

} // namespace hal

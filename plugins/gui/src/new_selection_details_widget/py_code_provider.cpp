#include "gui/new_selection_details_widget/py_code_provider.h"

//#include <QString>

#include <QDebug>

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

    QString PyCodeProvider::pyCodeGateClearPresetBehavior(u32 gateId)
    {
        return pyCodeGateType(gateId) + ".get_clear_preset_behavior()";
    }

    QString PyCodeProvider::pyCodeGateDataMap(u32 gateId)
    {
        const QString suffix = QString("get_data_map()");
        return buildPyCode(gateCodePrefix, suffix, gateId);
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

    QString PyCodeProvider::pyCodeNetName(u32 netId)
    {
        const QString suffix = "get_name()";

        return buildPyCode(netCodePrefix, suffix, netId);
    }

    QString PyCodeProvider::pyCodeNetType(u32 netId)
    {
        const QString prefix = QString(netCodePrefix).arg(netId);

        qDebug() << prefix;

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

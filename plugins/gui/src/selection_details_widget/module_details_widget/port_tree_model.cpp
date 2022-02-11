#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"

#include "gui/basic_tree_model/tree_item.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/enums.h"

#include <QDebug>

namespace hal
{
    ModulePinsTreeModel::ModulePinsTreeModel(QObject* parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QList<QVariant>() << "Name"
                                          << "Direction"
                                          << "Type"
                                          << "Connected Net");
        setModule(gNetlist->get_module_by_id(1));

        //connections
        connect(gNetlistRelay, &NetlistRelay::modulePortsChanged, this, &ModulePinsTreeModel::handleModulePortsChanged);
    }

    ModulePinsTreeModel::~ModulePinsTreeModel()
    {
        delete mRootItem;
    }

    void ModulePinsTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleId = -1;
        mPortGroupingToTreeItem.clear();
    }

    void ModulePinsTreeModel::setModule(Module* m)
    {
        clear();
        mModuleId = m->get_id();
        beginResetModel();

        for (PinGroup<ModulePin>* pinGroup : m->get_pin_groups())
        {
            //ignore empty pingroups
            if(pinGroup->empty())
                continue;

            ModulePin* firstPin = pinGroup->get_pins().front();
            QString pinGroupName;
            QString pinGroupDirection = QString::fromStdString(enum_to_string(firstPin->get_direction()));
            QString pinGroupType      = QString::fromStdString(enum_to_string(firstPin->get_type()));
            if (pinGroup->size() == 1)
            {
                pinGroupName = QString::fromStdString(firstPin->get_name());
            }
            else
            {
                pinGroupName = QString::fromStdString(pinGroup->get_name());
            }

            TreeItem* pinGroupItem = new TreeItem(QList<QVariant>() << pinGroupName << pinGroupDirection << pinGroupType << "");

            if (pinGroup->size() == 1)
            {
                pinGroupItem->setDataAtIndex(sNetColumn, QString::fromStdString(firstPin->get_net()->get_name()));
                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portSingleBit));
                //since a single-pin pingroup represents the pin itself, take the pinid
                pinGroupItem->setAdditionalData(keyId, firstPin->get_id());
            }
            else
            {
                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portMultiBit));
                pinGroupItem->setAdditionalData(keyId, pinGroup->get_id());
                for (ModulePin* pin : pinGroup->get_pins())
                {
                    TreeItem* pinItem = new TreeItem(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinGroupDirection << pinGroupType << QString::fromStdString(pin->get_net()->get_name()));
                    pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
                    pinItem->setAdditionalData(keyId, pin->get_id());
                    pinGroupItem->appendChild(pinItem);
                }
            }
            mRootItem->appendChild(pinGroupItem);
        }
        endResetModel();

        Q_EMIT numberOfPortsChanged(m->get_pins().size());
    }

    Net* ModulePinsTreeModel::getNetFromItem(TreeItem* item)
    {
        if (mModuleId == -1)    //no current module = no represented net
            return nullptr;

        itemType type = getTypeOfItem(item);
        if (type == itemType::portMultiBit)
            return nullptr;

        Module* m = gNetlist->get_module_by_id(mModuleId);
        if (!m)
            return nullptr;

        //std::string name = item->getData(sNameColumn).toString().toStdString();
        if (auto pinResult = m->get_pin_by_id(getIdOfItem(item)); pinResult.is_ok())
        {
            return pinResult.get()->get_net();
        }

        return nullptr;
    }

    int ModulePinsTreeModel::getRepresentedModuleId()
    {
        return mModuleId;
    }

    ModulePinsTreeModel::itemType ModulePinsTreeModel::getTypeOfItem(TreeItem* item)
    {
        return item->getAdditionalData(keyType).value<itemType>();
    }

    int ModulePinsTreeModel::getIdOfItem(TreeItem *item)
    {
        return item->getAdditionalData(keyId).toInt();
    }

    void ModulePinsTreeModel::handleModulePortsChanged(Module *m)
    {
        if ((int)m->get_id() == mModuleId)
            setModule(m);
    }
}    // namespace hal

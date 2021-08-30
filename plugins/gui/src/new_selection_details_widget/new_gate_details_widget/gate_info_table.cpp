#include "gui/new_selection_details_widget/new_gate_details_widget/gate_info_table.h"

#include "hal_core/utilities/enums.h"

#include "gui/gui_globals.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/new_selection_details_widget/py_code_provider.h"
#include "gui/user_action/action_rename_object.h"

#include <QMenu>
#include <QInputDialog>

namespace hal
{
    GateInfoTable::GateInfoTable(QWidget* parent) : GeneralTableWidget(parent)
    {
        mNameEntryContextMenu = new QMenu();
        mNameEntryContextMenu->addAction("changeName", std::bind(&GateInfoTable::changeName, this));
        mNameEntryContextMenu->addAction("copyName", std::bind(&GateInfoTable::copyName, this));
        mNameEntryContextMenu->addAction("pyCopyName", std::bind(&GateInfoTable::pyCopyName, this));

        mIdEntryContextMenu = new QMenu();
        mIdEntryContextMenu->addAction("copyId", std::bind(&GateInfoTable::copyId, this));

        mTypeEntryContextMenu = new QMenu();
        mTypeEntryContextMenu->addAction("copyType", std::bind(&GateInfoTable::copyType, this));
        mTypeEntryContextMenu->addAction("pyCopyType", std::bind(&GateInfoTable::pyCopyType, this));

        mPropertiesEntryContextMenu = new QMenu();
        mPropertiesEntryContextMenu->addAction("copyProperties", std::bind(&GateInfoTable::copyproperties, this));
        mPropertiesEntryContextMenu->addAction("pyCopyProperties", std::bind(&GateInfoTable::pyCopyproperties, this));

        mLocationEntryContextMenu = new QMenu();
        mLocationEntryContextMenu->addAction("copyLocation", std::bind(&GateInfoTable::copyLocation, this));
        mLocationEntryContextMenu->addAction("pyCopyLocation", std::bind(&GateInfoTable::pyCopyLocation, this));

        mModuleEntryContextMenu = new QMenu();
        mModuleEntryContextMenu->addAction("copyModule", std::bind(&GateInfoTable::copyModule, this));
        mModuleEntryContextMenu->addAction("pyCopyModule", std::bind(&GateInfoTable::pyCopyModule, this));

        mModuleDoubleClickedAction = std::bind(&GateInfoTable::navModule, this);

        connect(gNetlistRelay, &NetlistRelay::gateRemoved, this, &GateInfoTable::handleGateRemoved);
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &GateInfoTable::handleGateNameChanged);
        connect(gNetlistRelay, &NetlistRelay::gateLocationChanged, this, &GateInfoTable::handleGateLocationChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &GateInfoTable::handleModuleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &GateInfoTable::handleModuleGateAssigned);
    }

    void GateInfoTable::setGate(Gate* gate)
    {
        if(gNetlist->is_gate_in_netlist(gate))
        {
            mGate = gate;

            setRow("Name", name(), mNameEntryContextMenu);
            setRow("Id", id(), mIdEntryContextMenu);
            setRow("Type", type(), mTypeEntryContextMenu);
            setRow("Gate Type properties", properties(), mPropertiesEntryContextMenu);
            setRow("Location", location(), mLocationEntryContextMenu);
            setRow("Module", parentModule(), mModuleEntryContextMenu, mModuleDoubleClickedAction);

            adjustSize();
        }
    }
    
    QString GateInfoTable::name() const
    {
        return QString::fromStdString(mGate->get_name());
    }

    QString GateInfoTable::id() const
    {
        return QString::number(mGate->get_id());
    }

    QString GateInfoTable::type() const
    {
        QString gateType = "None";

        GateType* type = mGate->get_type();

        if(type)
            gateType = QString::fromStdString(type->get_name());

        return gateType;
    }

    QString GateInfoTable::properties() const
    {
        QString properties = "None";

        GateType* type = mGate->get_type();

        if(type)
        {
            properties.clear();

            for(hal::GateTypeProperty gtp : type->get_properties())
                properties.append(QString::fromStdString(enum_to_string(gtp)) + ", ");

            properties.chop(2);
        }
        
        return properties;
    }

    QString GateInfoTable::location() const
    {
        QString location = "N/A";

        i32 locationX = mGate->get_location_x();
        i32 locationY = mGate->get_location_y();

        if(locationX > 0 && locationY > 0)
            location = "X:" + QString::number(locationX) + " Y:" + QString::number(locationY);

        return location;
    }

    QString GateInfoTable::parentModule() const
    {
        Module* module = mGate->get_module();

        return QString::fromStdString(module->get_name()) + "[Id:" + QString::number(module->get_id()) + "]";
    }

    void GateInfoTable::changeName()
    {
        QString oldName = QString::fromStdString(mGate->get_name());
        QString prompt = "Change gate name";

        bool confirm;
        QString newName = QInputDialog::getText(this, prompt, "New name:", QLineEdit::Normal, oldName, &confirm);

        if (confirm)
        {
            ActionRenameObject* act = new ActionRenameObject(newName);
            act->setObject(UserActionObject(mGate->get_id(), UserActionObjectType::ObjectType::Gate));
            act->exec();
        }
    }

    void GateInfoTable::copyName() const
    {
        copyToClipboard(name());
    }

    void GateInfoTable::pyCopyName() const
    {
        copyToClipboard(PyCodeProvider::pyCodeGateName(mGate->get_id()));
    }

    void GateInfoTable::copyId() const
    {
        copyToClipboard(id());
    }

    void GateInfoTable::copyType() const
    {
        copyToClipboard(type());
    }

    void GateInfoTable::pyCopyType() const
    {
        copyToClipboard(PyCodeProvider::pyCodeGateType(mGate->get_id()));
    }

    void GateInfoTable::copyproperties() const
    {
        copyToClipboard(properties());
    }

    void GateInfoTable::pyCopyproperties() const
    {
        copyToClipboard(PyCodeProvider::pyCodeProperties(mGate->get_id()));
    }

    void GateInfoTable::copyLocation() const
    {
        copyToClipboard(location());
    }

    void GateInfoTable::pyCopyLocation() const
    {
        copyToClipboard(PyCodeProvider::pyCodeGateLocation(mGate->get_id()));
    }

    void GateInfoTable::copyModule() const
    {
        copyToClipboard(parentModule());
    }

    void GateInfoTable::pyCopyModule() const
    {
        copyToClipboard(PyCodeProvider::pyCodeGateModule(mGate->get_id()));
    }

    void GateInfoTable::navModule()
    {
        u32 parentModuleId = mGate->get_module()->get_id();

        gSelectionRelay->clear();
        gSelectionRelay->addModule(parentModuleId);
        gSelectionRelay->relaySelectionChanged(this);
    }

    void GateInfoTable::handleGateRemoved(Gate* gate)
    {
        if(mGate == gate)
        {
            mGate = nullptr;

            const QString notification("Displayed gate has been deleted.");

            setRow("Name", notification, nullptr);
            setRow("Id", notification, nullptr);
            setRow("Type", notification, nullptr);
            setRow("Gate Type properties", notification, nullptr);
            setRow("Location", notification, nullptr);
            setRow("Module", notification, nullptr, nullptr);

            adjustSize();
        }
    }

    void GateInfoTable::handleGateNameChanged(Gate* gate)
    {
        if(mGate == gate)
            refresh();
    }

    void GateInfoTable::handleGateLocationChanged(Gate* gate)
    {
        if(mGate == gate)
            refresh();
    }

    void GateInfoTable::handleModuleNameChanged(Module* module)
    {
        if(mGate->get_module() == module)
            refresh();
    }

    void GateInfoTable::handleModuleGateAssigned(Module* module, const u32 gateId)
    {
        Q_UNUSED(module)

        if(mGate->get_id() == gateId)
            refresh();
    }

    void GateInfoTable::refresh()
    {
        setGate(mGate);
    }
}

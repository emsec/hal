#include "gui/new_selection_details_widget/new_gate_details_widget/gate_info_table.h"

#include "hal_core/utilities/enums.h"

#include "gui/gui_globals.h"

#include "gui/new_selection_details_widget/py_code_provider.h"

#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

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

        setContent(11);
    }

    void GateInfoTable::update(u32 gateId)
    {
        mGate = gNetlist->get_gate_by_id(gateId);

        setRow("Name", name(), mNameEntryContextMenu);
        setRow("Id", id(), mIdEntryContextMenu);
        setRow("Type", type(), mTypeEntryContextMenu);
        setRow("Gate Type properties", properties(), mPropertiesEntryContextMenu);
        setRow("Location", location(), mLocationEntryContextMenu);
        setRow("Module", module(), mModuleEntryContextMenu, mModuleDoubleClickedAction);
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

    QString GateInfoTable::module() const
    {
        Module* module = mGate->get_module();

        return QString::fromStdString(module->get_name()) + "[Id:" + QString::number(module->get_id()) + "]";
    }

    void GateInfoTable::changeName()
    {
        qDebug() << "changeName()";
    }

    void GateInfoTable::copyName() const
    {
        QApplication::clipboard()->setText(name());
    }

    void GateInfoTable::pyCopyName() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeGateName(mGate->get_id()));
    }

    void GateInfoTable::copyId() const
    {
        QApplication::clipboard()->setText(id());
    }

    void GateInfoTable::copyType() const
    {
        QApplication::clipboard()->setText(type());
    }

    void GateInfoTable::pyCopyType() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeGateType(mGate->get_id()));
    }

    void GateInfoTable::copyproperties() const
    {
        QApplication::clipboard()->setText(properties());
    }

    void GateInfoTable::pyCopyproperties() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeProperties(mGate->get_id()));
    }

    void GateInfoTable::copyLocation() const
    {
        QApplication::clipboard()->setText(location());
    }

    void GateInfoTable::pyCopyLocation() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeGateLocation(mGate->get_id()));
    }

    void GateInfoTable::copyModule() const
    {
        QApplication::clipboard()->setText(module());
    }

    void GateInfoTable::pyCopyModule() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeGateModule(mGate->get_id()));
    }

    void GateInfoTable::navModule()
    {
        u32 parentModuleId = mGate->get_module()->get_id();

        gSelectionRelay->clear();
        gSelectionRelay->addModule(parentModuleId);
        gSelectionRelay->relaySelectionChanged(this);
    }
}

#include "gui/new_selection_details_widget/new_module_details_widget/module_info_table.h"

#include "hal_core/netlist/module.h"

#include "gui/new_selection_details_widget/py_code_provider.h"
#include "gui/gui_globals.h"

#include "gui/selection_details_widget/details_table_utilities.h"

#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal
{
    ModuleInfoTable::ModuleInfoTable(QWidget* parent) : GeneralTableWidget(parent)
    {
        mNameEntryContextMenu = new QMenu();
        mNameEntryContextMenu->addAction("changeName", std::bind(&ModuleInfoTable::changeName, this));
        mNameEntryContextMenu->addAction("copyName", std::bind(&ModuleInfoTable::copyName, this));
        mNameEntryContextMenu->addAction("pyCopyName", std::bind(&ModuleInfoTable::pyCopyName, this));

        mIdEntryContextMenu = new QMenu();
        mIdEntryContextMenu->addAction("copyId", std::bind(&ModuleInfoTable::copyId, this));

        mTypeEntryContextMenu = new QMenu();
        mTypeEntryContextMenu->addAction("changeType", std::bind(&ModuleInfoTable::changeType, this));
        mTypeEntryContextMenu->addAction("copyType", std::bind(&ModuleInfoTable::copyType, this));
        mTypeEntryContextMenu->addAction("pyCopyType", std::bind(&ModuleInfoTable::pyCopyType, this));

        mModuleEntryContextMenu = new QMenu();
        mModuleEntryContextMenu->addAction("copyModule", std::bind(&ModuleInfoTable::copyModule, this));
        mModuleEntryContextMenu->addAction("pyCopyModule", std::bind(&ModuleInfoTable::pyCopyModule, this));

        mNumOfGatesContextMenu = new QMenu();
        mNumOfGatesContextMenu->addAction("copyNumOfGates", std::bind(&ModuleInfoTable::copyNumberOfGates, this));

        mNumOfSubmodulesContextMenu = new QMenu();
        mNumOfSubmodulesContextMenu->addAction("copyNumOfSubmodules", std::bind(&ModuleInfoTable::copyNumberOfSubmodules, this));

        mNumOfNetsContextMenu = new QMenu();
        mNumOfNetsContextMenu->addAction("copyNumOfNets", std::bind(&ModuleInfoTable::copyNumberOfNets, this));

        mModuleDoubleClickedAction = std::bind(&ModuleInfoTable::navModule, this);

        //debug - remove later
        setContent(1);
    }

    void ModuleInfoTable::update(u32 moduleId)
    {
        mModule = gNetlist->get_module_by_id(moduleId);

        setRow("Name", name(), mNameEntryContextMenu);
        setRow("Id", id(), mIdEntryContextMenu);
        setRow("Type", type(), mTypeEntryContextMenu);
        setRow("Parent Module", module(), mModuleEntryContextMenu, mModuleDoubleClickedAction);
        setRow("No. of Gates", numberOfGates(), mNumOfGatesContextMenu);
        setRow("No. of Submodules", numberOfSubModules(), mNumOfSubmodulesContextMenu);
        setRow("No. of Nets", numberOfNets(), mNumOfNetsContextMenu);
    }

    QString ModuleInfoTable::name() const
    {
        return QString::fromStdString(mModule->get_name());
    }

    QString ModuleInfoTable::id() const
    {
        return QString::number(mModule->get_id());
    }

    QString ModuleInfoTable::type() const
    {
        QString type = QString::fromStdString(mModule->get_type());

        if(type.isEmpty())
            type = "None";

        return type;
    }

    QString ModuleInfoTable::module() const
    {
        QString parentModule = "None";

        Module* module = mModule->get_parent_module();

        if(module)
            parentModule = QString::fromStdString(module->get_name()) + "[Id:" + QString::number(module->get_id()) + "]";

        return parentModule;
    }

    QString ModuleInfoTable::numberOfGates() const
    {
        QString numOfGates = "";

        int numOfAllChilds = mModule->get_gates(nullptr, true).size();
        int numOfDirectChilds = mModule->get_gates(nullptr, false).size();
        int numOfChildsInModules = numOfAllChilds - numOfDirectChilds;

        numOfGates.append(QString::number(numOfAllChilds));

        if(numOfChildsInModules > 0)
            numOfGates.append(" (" + QString::number(numOfDirectChilds) + " direct members and " + QString::number(numOfChildsInModules) +" in submodules)");

        return numOfGates;
    }

    QString ModuleInfoTable::numberOfSubModules() const
    {
        return QString::number(mModule->get_submodules(nullptr, true).size());
    }

    QString ModuleInfoTable::numberOfNets() const
    {
        return QString::number(mModule->get_internal_nets().size());
    }

    void ModuleInfoTable::changeName()
    {
        qDebug() << "changeName()";
    }

    void ModuleInfoTable::copyName() const
    {
        QApplication::clipboard()->setText(name());
    }

    void ModuleInfoTable::pyCopyName() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeModuleName(mModule->get_id()));
    }

    void ModuleInfoTable::copyId() const
    {
        QApplication::clipboard()->setText(id());
    }

    void ModuleInfoTable::changeType() const
    {
        qDebug() << "changeType()";
    }

    void ModuleInfoTable::copyType() const
    {
        QApplication::clipboard()->setText(type());
    }

    void ModuleInfoTable::pyCopyType() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeModuleType(mModule->get_id()));
    }

    void ModuleInfoTable::copyModule() const
    {
        QApplication::clipboard()->setText(module());
    }

    void ModuleInfoTable::pyCopyModule() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeModuleModule(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfGates() const
    {
        QApplication::clipboard()->setText(numberOfGates());
    }

    void ModuleInfoTable::copyNumberOfSubmodules() const
    {
        QApplication::clipboard()->setText(numberOfSubModules());
    }

    void ModuleInfoTable::copyNumberOfNets() const
    {
        QApplication::clipboard()->setText(numberOfNets());
    }

    void ModuleInfoTable::navModule()
    {
        Module* parentModule = mModule->get_parent_module();

        if(parentModule != nullptr)
        {
            u32 parentModuleId = parentModule->get_id();

            gSelectionRelay->clear();
            gSelectionRelay->addModule(parentModuleId);
            gSelectionRelay->relaySelectionChanged(this);
        }
    }
}

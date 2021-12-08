#include "gui/selection_details_widget/module_details_widget/module_info_table.h"

#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/python/py_code_provider.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_type.h"

#include <QMenu>
#include <QInputDialog>

namespace hal
{
    const QString ModuleInfoTable::nameRowKey = "Name";
    const QString ModuleInfoTable::idRowKey = "ID";
    const QString ModuleInfoTable::typeRowKey = "Module Type";
    const QString ModuleInfoTable::moduleRowKey = "Parent module";
    const QString ModuleInfoTable::noOfGatesRowKey = "No. of Gates";
    const QString ModuleInfoTable::noOfModulesRowKey = "No. of Submodules";
    const QString ModuleInfoTable::noOfNetsRowKey = "No. of Nets";

    ModuleInfoTable::ModuleInfoTable(QWidget* parent) : GeneralTableWidget(parent), mModule(nullptr)
    {
        mNameEntryContextMenu = new QMenu();
        mNameEntryContextMenu->addAction("Extract module name as plain text", std::bind(&ModuleInfoTable::copyName, this));
        mNameEntryContextMenu->addSection("Misc");
        mNameEntryContextMenu->addAction("Change module name", std::bind(&ModuleInfoTable::changeName, this));
        mNameEntryContextMenu->addSection("Python");
        mNameEntryContextMenu->addAction(QIcon(":/icons/python"), "Extract module name as phyton code", std::bind(&ModuleInfoTable::pyCopyName, this));

        mIdEntryContextMenu = new QMenu();
        mIdEntryContextMenu->addAction("Extract module ID as plain text", std::bind(&ModuleInfoTable::copyId, this));

        mTypeEntryContextMenu = new QMenu();
        mTypeEntryContextMenu->addAction("Extract module type as plain text", std::bind(&ModuleInfoTable::copyType, this));
        mTypeEntryContextMenu->addSection("Misc");
        mTypeEntryContextMenu->addAction("Change module type", std::bind(&ModuleInfoTable::changeType, this));
        mTypeEntryContextMenu->addSection("Python");
        mTypeEntryContextMenu->addAction(QIcon(":/icons/python"), "Extract module type as python code", std::bind(&ModuleInfoTable::pyCopyType, this));

        mModuleEntryContextMenu = new QMenu();
        mModuleEntryContextMenu->addAction("Extract parent module name plain text", std::bind(&ModuleInfoTable::copyModule, this));
        mModuleEntryContextMenu->addSection("Python");
        mModuleEntryContextMenu->addAction(QIcon(":/icons/python"), "Extract parent module name python code", std::bind(&ModuleInfoTable::pyCopyModule, this));

        mNumOfGatesContextMenu = new QMenu();
        mNumOfGatesContextMenu->addAction("Extract number of gates as plain text", std::bind(&ModuleInfoTable::copyNumberOfGates, this));

        mNumOfSubmodulesContextMenu = new QMenu();
        mNumOfSubmodulesContextMenu->addAction("Extract number of submodule as plain text", std::bind(&ModuleInfoTable::copyNumberOfSubmodules, this));

        mNumOfNetsContextMenu = new QMenu();
        mNumOfNetsContextMenu->addAction("Extract number of nets to clipboard", std::bind(&ModuleInfoTable::copyNumberOfNets, this));

        mModuleDoubleClickedAction = std::bind(&ModuleInfoTable::navModule, this);

        connect(gNetlistRelay, &NetlistRelay::moduleRemoved, this, &ModuleInfoTable::handleModuleRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &ModuleInfoTable::handleModuleChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleParentChanged, this, &ModuleInfoTable::handleModuleChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleTypeChanged, this, &ModuleInfoTable::handleModuleChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleAdded, this, &ModuleInfoTable::handleSubmoduleChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &ModuleInfoTable::handleSubmoduleChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleInfoTable::handleGateChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleInfoTable::handleGateChanged);
        connect(gNetlistRelay, &NetlistRelay::netSourceAdded, this, &ModuleInfoTable::handleNetChaned);
        connect(gNetlistRelay, &NetlistRelay::netSourceRemoved, this, &ModuleInfoTable::handleNetChaned);
        connect(gNetlistRelay, &NetlistRelay::netDestinationAdded, this, &ModuleInfoTable::handleNetChaned);
        connect(gNetlistRelay, &NetlistRelay::netDestinationRemoved, this, &ModuleInfoTable::handleNetChaned);
    }

    void ModuleInfoTable::setModule(hal::Module* module)
    {
        if(gNetlist->is_module_in_netlist(module))
        {
            mModule = module;

            setRow(nameRowKey, name(), mNameEntryContextMenu);
            setRow(idRowKey, id(), mIdEntryContextMenu);
            setRow(typeRowKey, type(), mTypeEntryContextMenu);
            setRow(moduleRowKey, parentModule(), mModuleEntryContextMenu, mModuleDoubleClickedAction);
            setRow(noOfGatesRowKey, numberOfGates(), mNumOfGatesContextMenu);
            setRow(noOfModulesRowKey, numberOfSubModules(), mNumOfSubmodulesContextMenu);
            setRow(noOfNetsRowKey, numberOfNets(), mNumOfNetsContextMenu);
            
            adjustSize();
        }
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

    QString ModuleInfoTable::parentModule() const
    {
        QString parentModule = "None";

        Module* module = mModule->get_parent_module();

        if(module)
            parentModule = QString::fromStdString(module->get_name()) + "[ID:" + QString::number(module->get_id()) + "]";

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
        QString oldName = QString::fromStdString(mModule->get_name());
        QString prompt = "Change module name";

        bool confirm;
        QString newName = QInputDialog::getText(this, prompt, "New name:", QLineEdit::Normal, oldName, &confirm);

        if (confirm)
        {
            ActionRenameObject* act = new ActionRenameObject(newName);
            act->setObject(UserActionObject(mModule->get_id(), UserActionObjectType::ObjectType::Module));
            act->exec();
        }
    }

    void ModuleInfoTable::copyName() const
    {
        copyToClipboard(name());
    }

    void ModuleInfoTable::pyCopyName() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleName(mModule->get_id()));
    }

    void ModuleInfoTable::copyId() const
    {
        copyToClipboard(id());
    }

    void ModuleInfoTable::changeType()
    {
        const QString type = QString::fromStdString(mModule->get_type());
        bool confirm;

        const QString newType = QInputDialog::getText(this, "Change module type", "New type:", QLineEdit::Normal, type, &confirm);

        if (confirm)
        {
            ActionSetObjectType* act = new ActionSetObjectType(newType);
            act->setObject(UserActionObject(mModule->get_id(),UserActionObjectType::Module));
            act->exec();
        }
    }

    void ModuleInfoTable::copyType() const
    {
        copyToClipboard(type());
    }

    void ModuleInfoTable::pyCopyType() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleType(mModule->get_id()));
    }

    void ModuleInfoTable::copyModule() const
    {
        copyToClipboard(parentModule());
    }

    void ModuleInfoTable::pyCopyModule() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleModule(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfGates() const
    {
        copyToClipboard(numberOfGates());
    }

    void ModuleInfoTable::copyNumberOfSubmodules() const
    {
        copyToClipboard(numberOfSubModules());
    }

    void ModuleInfoTable::copyNumberOfNets() const
    {
        copyToClipboard(numberOfNets());
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

    void ModuleInfoTable::refresh()
    {
        setModule(mModule);
    }

    void ModuleInfoTable::handleModuleRemoved(Module* module)
    {
        if(mModule == module)
        {
            mModule = nullptr;

            const QString notification("Displayed module has been removed.");

            setRow(nameRowKey, notification, nullptr);
            setRow(idRowKey, notification, nullptr);
            setRow(typeRowKey, notification, nullptr);
            setRow(moduleRowKey, notification, nullptr, nullptr);
            setRow(noOfGatesRowKey, notification, nullptr);
            setRow(noOfModulesRowKey, notification, nullptr);
            setRow(noOfNetsRowKey, notification, nullptr);

            adjustSize();
        }
    }

    void ModuleInfoTable::handleModuleChanged(Module* module)
    {
        if(mModule == module)
            refresh();
    }

    void ModuleInfoTable::handleSubmoduleChanged(Module* parentModule, u32 affectedModuleId)
    {
        Q_UNUSED(affectedModuleId);

        if(!mModule)
            return;

        if(mModule == parentModule || mModule->contains_module(parentModule))
            refresh();
    }

    void ModuleInfoTable::handleGateChanged(Module* parentModule, u32 affectedGateId)
    {
        Q_UNUSED(affectedGateId);

        if(!mModule)
            return;

        if(mModule == parentModule || mModule->contains_module(parentModule))
            refresh();
    }

    void ModuleInfoTable::handleNetChaned(Net* net, u32 affectedGateId)
    {
        Q_UNUSED(net);

        if(!mModule)
            return;

        Gate* affectedGate = gNetlist->get_gate_by_id(affectedGateId);
        std::vector<Gate*> allChildGates = mModule->get_gates(nullptr, true);

        if(std::find(std::begin(allChildGates), std::end(allChildGates), affectedGate) != std::end(allChildGates))
            refresh();
    }
}

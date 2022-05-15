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
    const QString ModuleInfoTable::typeRowKey = "Type";
    const QString ModuleInfoTable::moduleRowKey = "Parent";
    const QString ModuleInfoTable::noOfGatesRowKey = "No. of Gates";
    const QString ModuleInfoTable::noOfModulesRowKey = "Number of submodules";
    const QString ModuleInfoTable::noOfNetsRowKey = "Number of nets";
    const QString ModuleInfoTable::noOfPinsKey = "Number of pins";
    const QString ModuleInfoTable::noOfPinGroupsKey = "Number of pin groups";
    const QString ModuleInfoTable::noOfInputNetsKey = "Number of inputs";
    const QString ModuleInfoTable::noOfOutputNetsKey = "Number of outputs";
    const QString ModuleInfoTable::noOfInternalNetsKey = "Number of internal nets";
    const QString ModuleInfoTable::isTopModuleKey = "Is top module?";

    ModuleInfoTable::ModuleInfoTable(QWidget* parent) : GeneralTableWidget(parent), mModule(nullptr), mPyIcon(":/icons/python")
    {
        mNameEntryContextMenu = new QMenu();
        mNameEntryContextMenu->addAction("Name to clipboard", std::bind(&ModuleInfoTable::copyName, this));
        mNameEntryContextMenu->addSection("Misc");
        mNameEntryContextMenu->addAction("Change name", std::bind(&ModuleInfoTable::changeName, this));
        mNameEntryContextMenu->addSection("Python");
        mNameEntryContextMenu->addAction(mPyIcon, "Get name", std::bind(&ModuleInfoTable::pyCopyName, this));

        mIdEntryContextMenu = new QMenu();
        mIdEntryContextMenu->addAction("ID to clipboard", std::bind(&ModuleInfoTable::copyId, this));
        mIdEntryContextMenu->addSection("Python");
        mIdEntryContextMenu->addAction(mPyIcon, "Get ID", std::bind(&ModuleInfoTable::pyCopyId, this));

        mTypeEntryContextMenu = new QMenu();
        mTypeEntryContextMenu->addAction("Type to clipboard", std::bind(&ModuleInfoTable::copyType, this));
        mTypeEntryContextMenu->addSection("Misc");
        mTypeEntryContextMenu->addAction("Change type", std::bind(&ModuleInfoTable::changeType, this));
        mTypeEntryContextMenu->addSection("Python");
        mTypeEntryContextMenu->addAction(mPyIcon, "Get type", std::bind(&ModuleInfoTable::pyCopyType, this));

        mModuleEntryContextMenu = new QMenu();
        mModuleEntryContextMenu->addAction("Parent name to clipboard", std::bind(&ModuleInfoTable::copyModule, this));
        mModuleEntryContextMenu->addSection("Python");
        mModuleEntryContextMenu->addAction(mPyIcon, "Get parent", std::bind(&ModuleInfoTable::pyCopyModule, this));

        mNumOfGatesContextMenu = new QMenu();
        mNumOfGatesContextMenu->addAction("Number of gates to clipboard", std::bind(&ModuleInfoTable::copyNumberOfGates, this));

        mNumOfSubmodulesContextMenu = new QMenu();
        mNumOfSubmodulesContextMenu->addAction("Number of submodules to clipboard", std::bind(&ModuleInfoTable::copyNumberOfSubmodules, this));
        mNumOfSubmodulesContextMenu->addAction(mPyIcon, "Get submodules", std::bind(&ModuleInfoTable::pyCopyGetSubmodules, this));

        mNumOfNetsContextMenu = new QMenu();
        mNumOfNetsContextMenu->addAction("Number of nets to clipboard", std::bind(&ModuleInfoTable::copyNumberOfNets, this));
        mNumOfNetsContextMenu->addAction(mPyIcon, "Get nets", std::bind(&ModuleInfoTable::pyCopyGetNets, this));

        mNumOfPinsContextMenu = new QMenu;
        mNumOfPinsContextMenu->addAction("Number of pins to clipboard", std::bind(&ModuleInfoTable::copyNumberOfPins, this));
        mNumOfPinsContextMenu->addAction(mPyIcon, "Get pins", std::bind(&ModuleInfoTable::pyCopyGetPins, this));

        mNumOfPinGroupsContextMenu = new QMenu;
        mNumOfPinGroupsContextMenu->addAction("Number of pin groups to clipboard", std::bind(&ModuleInfoTable::copyNumberOfPinGroups, this));
        mNumOfPinGroupsContextMenu->addAction(mPyIcon, "Get pin groups", std::bind(&ModuleInfoTable::pyCopyGetPinGroups, this));

        mNumOfInputNetsContextMenu = new QMenu;//making this widget the parent changes the context menu's background color...
        mNumOfInputNetsContextMenu->addAction("Number of input nets to clipboard", std::bind(&ModuleInfoTable::copyNumberOfInputs, this));
        mNumOfInputNetsContextMenu->addAction(mPyIcon, "Get input nets", std::bind(&ModuleInfoTable::pyCopyGetInputNets,this));

        mNumOfOutputNetsContextMenu = new QMenu;
        mNumOfOutputNetsContextMenu->addAction("Number of output nets to clipboard", std::bind(&ModuleInfoTable::copyNumberOfOutputs, this));
        mNumOfOutputNetsContextMenu->addAction(mPyIcon, "Get output nets", std::bind(&ModuleInfoTable::pyCopyGetOutputNets, this));

        mNumOfInternalNetsContextMenu = new QMenu;
        mNumOfInternalNetsContextMenu->addAction("Number of internal nets to clipboard", std::bind(&ModuleInfoTable::copyNumberOfInternalNets, this));
        mNumOfInternalNetsContextMenu->addAction(mPyIcon, "Get internal nets", std::bind(&ModuleInfoTable::pyCopyGetInternalNets, this));

        mIsTopModuleContextMenu = new QMenu;
        mIsTopModuleContextMenu->addAction(mPyIcon, "Check if module is top module", std::bind(&ModuleInfoTable::pyCopyIsTopModule, this));

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
            setRow(noOfPinsKey, numberOfPins(), mNumOfPinsContextMenu);
            setRow(noOfPinGroupsKey, numberOfPinGroups(), mNumOfPinGroupsContextMenu);
            setRow(noOfInputNetsKey, numberOfInputNets(), mNumOfInputNetsContextMenu);
            setRow(noOfOutputNetsKey, numberOfOutputNets(), mNumOfOutputNetsContextMenu);
            setRow(noOfInternalNetsKey, numberOfInternalNets(), mNumOfInternalNetsContextMenu);
            setRow(isTopModuleKey, isTopModule(), mIsTopModuleContextMenu);
            
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

    QString ModuleInfoTable::numberOfPins() const
    {
        return QString::number(mModule->get_pins().size());
    }

    QString ModuleInfoTable::numberOfPinGroups() const
    {
        return QString::number(mModule->get_pin_groups().size());
    }

    QString ModuleInfoTable::numberOfInputNets() const
    {
        return QString::number(mModule->get_input_nets().size());
    }

    QString ModuleInfoTable::numberOfOutputNets() const
    {
        return QString::number(mModule->get_output_nets().size());
    }

    QString ModuleInfoTable::numberOfInternalNets() const
    {
        return QString::number(mModule->get_internal_nets().size());
    }

    QString ModuleInfoTable::isTopModule() const
    {
        return mModule->is_top_module() ? "True" : "False";
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

    void ModuleInfoTable::pyCopyId() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleId(mModule->get_id()));
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

    void ModuleInfoTable::pyCopyGetSubmodules() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleSubmodules(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfNets() const
    {
        copyToClipboard(numberOfNets());
    }

    void ModuleInfoTable::pyCopyGetNets() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleNets(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfPins() const
    {
        copyToClipboard(numberOfPins());
    }

    void ModuleInfoTable::copyNumberOfPinGroups() const
    {
        copyToClipboard(numberOfPinGroups());
    }

    void ModuleInfoTable::pyCopyGetPinGroups() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModulePinGroups(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfInputs() const
    {
        copyToClipboard(numberOfInputNets());
    }

    void ModuleInfoTable::pyCopyGetInputNets() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleInputNets(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfOutputs() const
    {
        copyToClipboard(numberOfOutputNets());
    }

    void ModuleInfoTable::pyCopyGetOutputNets() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleOutputNets(mModule->get_id()));
    }

    void ModuleInfoTable::copyNumberOfInternalNets() const
    {
        copyToClipboard(numberOfInternalNets());
    }

    void ModuleInfoTable::pyCopyGetInternalNets() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleInternalNets(mModule->get_id()));
    }

    void ModuleInfoTable::pyCopyIsTopModule() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModuleIsTopModule(mModule->get_id()));
    }

    void ModuleInfoTable::pyCopyGetPins() const
    {
        copyToClipboard(PyCodeProvider::pyCodeModulePins(mModule->get_id()));
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

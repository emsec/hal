#include "gui/selection_details_widget/net_details_widget/net_info_table.h"

#include "gui/gui_globals.h"
#include "gui/python/py_code_provider.h"
#include "gui/user_action/action_rename_object.h"
#include "hal_core/netlist/net.h"

#include <QInputDialog>
#include <QMenu>

namespace hal
{
    const QString NetInfoTable::nameRowKey    = "Name";
    const QString NetInfoTable::idRowKey      = "ID";
    const QString NetInfoTable::typeRowKey    = "Type";
    const QString NetInfoTable::noOfSrcRowKey = "No. of Sources";
    const QString NetInfoTable::noOfDstRowKey = "No. of Destinations";

    NetInfoTable::NetInfoTable(QWidget* parent) : GeneralTableWidget(parent), mNet(nullptr)
    {
        mNameEntryContextMenu = new QMenu();
        mNameEntryContextMenu->addAction("Extract net name as plain text", std::bind(&NetInfoTable::copyName, this));
        mNameEntryContextMenu->addSection("Misc");
        mNameEntryContextMenu->addAction("Change net name", std::bind(&NetInfoTable::changeName, this));
        mNameEntryContextMenu->addSection("Phyton");
        mNameEntryContextMenu->addAction(QIcon(":/icons/python"), "Extract net name as phyton code", std::bind(&NetInfoTable::pyCopyName, this));

        mIdEntryContextMenu = new QMenu();
        mIdEntryContextMenu->addAction("Extract net ID as plain text", std::bind(&NetInfoTable::copyId, this));

        mTypeEntryContextMenu = new QMenu();
        mTypeEntryContextMenu->addAction("Extract net type as plain text", std::bind(&NetInfoTable::copyType, this));
        mTypeEntryContextMenu->addSection("Python");
        mTypeEntryContextMenu->addAction(QIcon(":/icons/python"), "Extract net type as python code", std::bind(&NetInfoTable::pyCopyType, this));

        mNumSrcsEntryContextMenu = new QMenu();
        mNumSrcsEntryContextMenu->addAction("Copy number of sources as plain text", std::bind(&NetInfoTable::copyNumberOfSrcs, this));

        mNumDstsEntryContextMenu = new QMenu();
        mNumDstsEntryContextMenu->addAction("Copy number of destinations as plain text", std::bind(&NetInfoTable::copyNumberOfDsts, this));

        connect(gNetlistRelay, &NetlistRelay::netRemoved, this, &NetInfoTable::handleNetRemoved);
        connect(gNetlistRelay, &NetlistRelay::netNameChanged, this, &NetInfoTable::handleNetNameChanged);
        connect(gNetlistRelay, &NetlistRelay::netSourceAdded, this, &NetInfoTable::handleSrcDstChanged);
        connect(gNetlistRelay, &NetlistRelay::netSourceRemoved, this, &NetInfoTable::handleSrcDstChanged);
        connect(gNetlistRelay, &NetlistRelay::netDestinationAdded, this, &NetInfoTable::handleSrcDstChanged);
        connect(gNetlistRelay, &NetlistRelay::netDestinationRemoved, this, &NetInfoTable::handleSrcDstChanged);
        connect(gNetlistRelay, &NetlistRelay::netlistMarkedGlobalInput, this, &NetInfoTable::handleNetTypeChanged);
        connect(gNetlistRelay, &NetlistRelay::netlistUnmarkedGlobalInput, this, &NetInfoTable::handleNetTypeChanged);
        connect(gNetlistRelay, &NetlistRelay::netlistMarkedGlobalOutput, this, &NetInfoTable::handleNetTypeChanged);
        connect(gNetlistRelay, &NetlistRelay::netlistUnmarkedGlobalOutput, this, &NetInfoTable::handleNetTypeChanged);
    }

    void NetInfoTable::setNet(hal::Net* net)
    {
        if (gNetlist->is_net_in_netlist(net))
        {
            mNet = net;

            setRow(nameRowKey, name(), mNameEntryContextMenu);
            setRow(idRowKey, id(), mIdEntryContextMenu);
            setRow(typeRowKey, type(), mTypeEntryContextMenu);
            setRow(noOfSrcRowKey, numberOfSrcs(), mNumSrcsEntryContextMenu);
            setRow(noOfDstRowKey, numberOfDsts(), mNumDstsEntryContextMenu);

            adjustSize();
        }
    }

    QString NetInfoTable::name() const
    {
        return QString::fromStdString(mNet->get_name());
    }

    QString NetInfoTable::id() const
    {
        return QString::number(mNet->get_id());
    }

    QString NetInfoTable::type() const
    {
        QString type = "";

        if (mNet->is_global_input_net())
            type = "Global input";
        else if (mNet->is_global_output_net())
            type = "Global output";
        else if (mNet->is_unrouted())
            type = "Unrouted";
        else
            type = "Internal";

        return type;
    }

    QString NetInfoTable::numberOfSrcs() const
    {
        return QString::number(mNet->get_num_of_sources());
    }

    QString NetInfoTable::numberOfDsts() const
    {
        return QString::number(mNet->get_num_of_destinations());
    }

    void NetInfoTable::changeName()
    {
        QString oldName = QString::fromStdString(mNet->get_name());
        QString prompt  = "Change net name";

        bool confirm;
        QString newName = QInputDialog::getText(this, prompt, "New name:", QLineEdit::Normal, oldName, &confirm);

        if (confirm)
        {
            ActionRenameObject* act = new ActionRenameObject(newName);
            act->setObject(UserActionObject(mNet->get_id(), UserActionObjectType::ObjectType::Net));
            act->exec();
        }
    }

    void NetInfoTable::copyName() const
    {
        copyToClipboard(name());
    }

    void NetInfoTable::pyCopyName() const
    {
        copyToClipboard(PyCodeProvider::pyCodeNetName(mNet->get_id()));
    }

    void NetInfoTable::copyId() const
    {
        copyToClipboard(id());
    }

    void NetInfoTable::copyType() const
    {
        copyToClipboard(type());
    }

    void NetInfoTable::pyCopyType() const
    {
        copyToClipboard(PyCodeProvider::pyCodeNetType(mNet->get_id()));
    }

    void NetInfoTable::copyNumberOfSrcs() const
    {
        copyToClipboard(numberOfSrcs());
    }

    void NetInfoTable::copyNumberOfDsts() const
    {
        copyToClipboard(numberOfDsts());
    }

    void NetInfoTable::handleNetRemoved(Net* net)
    {
        if (mNet == net)
        {
            mNet = nullptr;

            const QString notification("Displayed net has been removed.");

            setRow(nameRowKey, notification, nullptr);
            setRow(idRowKey, notification, nullptr);
            setRow(typeRowKey, notification, nullptr);
            setRow(noOfSrcRowKey, notification, nullptr);
            setRow(noOfDstRowKey, notification, nullptr);

            adjustSize();
        }
    }

    void NetInfoTable::handleNetNameChanged(Net* net)
    {
        if (mNet == net)
            refresh();
    }

    void NetInfoTable::handleNetTypeChanged(Netlist* netlist, const u32 netId)
    {
        Q_UNUSED(netlist)

        if(!mNet)
            return;

        if (mNet->get_id() == netId)
            refresh();
    }

    void NetInfoTable::handleSrcDstChanged(Net* net, u32 srcDstGateId)
    {
        Q_UNUSED(srcDstGateId)

        if (mNet == net)
            refresh();
    }

    void NetInfoTable::refresh()
    {
        setNet(mNet);
    }
}    // namespace hal

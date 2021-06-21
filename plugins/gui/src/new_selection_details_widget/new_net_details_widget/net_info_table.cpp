#include "gui/new_selection_details_widget/new_net_details_widget/net_info_table.h"

#include "hal_core/netlist/net.h"

#include "gui/gui_globals.h"
#include "gui/new_selection_details_widget/py_code_provider.h"

#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal
{
    NetInfoTable::NetInfoTable(QWidget* parent) : GeneralTableWidget(parent)
    {
        mNameEntryContextMenu = new QMenu();
        mNameEntryContextMenu->addAction("changeName", std::bind(&NetInfoTable::changeName, this));
        mNameEntryContextMenu->addAction("copyName", std::bind(&NetInfoTable::copyName, this));
        mNameEntryContextMenu->addAction("pyCopyName", std::bind(&NetInfoTable::pyCopyName, this));

        mIdEntryContextMenu = new QMenu();
        mIdEntryContextMenu->addAction("copyId", std::bind(&NetInfoTable::copyId, this));

        mTypeEntryContextMenu = new QMenu();
        mTypeEntryContextMenu->addAction("copyType", std::bind(&NetInfoTable::copyType, this));
        mTypeEntryContextMenu->addAction("pyCopyType", std::bind(&NetInfoTable::pyCopyType, this));

        mNumSrcsEntryContextMenu = new QMenu();
        mNumSrcsEntryContextMenu->addAction("copyNumSrcs", std::bind(&NetInfoTable::copyNumberOfSrcs, this));

        mNumDstsEntryContextMenu = new QMenu();
        mNumDstsEntryContextMenu->addAction("copyNumDsts", std::bind(&NetInfoTable::copyNumberOfDsts, this));

        setContent(17);
    }

    void NetInfoTable::update(u32 netId)
    {
        mNet = gNetlist->get_net_by_id(netId);

        setRow("Name", name(), mNameEntryContextMenu);
        setRow("Id", id(), mIdEntryContextMenu);
        setRow("Type", type(), mTypeEntryContextMenu);
        setRow("No. of Sources", numberOfSrcs(), mNumSrcsEntryContextMenu);
        setRow("No. of Destinations", numberOfDsts(), mNumDstsEntryContextMenu);
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
        QString type = "Internal";

        if(mNet->is_global_input_net())
            type = "Global input";
        else if(mNet->is_global_output_net())
            type = "Global output";
        else if(mNet->is_unrouted())
            type = "Unrouted";
        
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
        qDebug() << "changeName";
    }

    void NetInfoTable::copyName() const
    {
        QApplication::clipboard()->setText(name());
    }

    void NetInfoTable::pyCopyName() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeNetName(mNet->get_id()));
    }

    void NetInfoTable::copyId() const
    {
        QApplication::clipboard()->setText(id());
    }

    void NetInfoTable::copyType() const
    {
        QApplication::clipboard()->setText(type());
    }
    
    void NetInfoTable::pyCopyType() const
    {
        QApplication::clipboard()->setText(PyCodeProvider::pyCodeNetType(mNet->get_id()));
    }
    
    void NetInfoTable::copyNumberOfSrcs() const
    {
        QApplication::clipboard()->setText(numberOfSrcs());
    }

    void NetInfoTable::copyNumberOfDsts() const
    {
        QApplication::clipboard()->setText(numberOfDsts());
    }
}

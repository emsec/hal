#include "gui/selection_details_widget/net_details_tab_widget.h"

#include "gui/selection_details_widget/details_frame_widget.h"
#include "gui/selection_details_widget/net_details_widget/net_info_table.h"

#include "gui/selection_details_widget/net_details_widget/net_endpoint_table.h"
#include "gui/selection_details_widget/net_details_widget/endpoint_table_model.h"

#include "gui/selection_details_widget/net_details_widget/net_module_table.h"
#include "gui/selection_details_widget/net_details_widget/module_table_model.h"

#include "hal_core/netlist/net.h"

namespace hal
{
    NetDetailsTabWidget::NetDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(SelectionDetailsIconProvider::NetIcon);

        //create all widgets and replace the qWidgets in the frames with them
        mNetInfoTable = new NetInfoTable(this);

        //general tab
        mNetInformationFrame = new DetailsFrameWidget(mNetInfoTable, "Net Information", this);
        mGroupingsOfItemTable = new GroupingsOfItemWidget;
        mGroupingsFrame = new DetailsFrameWidget(mGroupingsOfItemTable, "Groupings", this);
        connect(mGroupingsOfItemTable,  &GroupingsOfItemWidget::updateText, mGroupingsFrame, &DetailsFrameWidget::setText);

        ModuleTableModel* moduleModel = new ModuleTableModel(this);
        mNetModuleTable = new NetModuleTable(moduleModel, this);

        mModulesFrame = new DetailsFrameWidget(mNetModuleTable, "Modules", this);

        QList<DetailsFrameWidget*> framesGeneralTab({mNetInformationFrame, mGroupingsFrame, mModulesFrame});
        addTab("General", framesGeneralTab);

        //sources / destinations tab

        EndpointTableModel* sourceModel = new EndpointTableModel(EndpointTableModel::Type::source, this);
        mSourcesTable = new NetEndpointTable(sourceModel, this);

        EndpointTableModel* destinationModel = new EndpointTableModel(EndpointTableModel::Type::destination, this);
        mDestinationsTable = new NetEndpointTable(destinationModel, this);

        mSourcesFrame = new DetailsFrameWidget(mSourcesTable, "Sources", this);
        mDestinationsFrame = new DetailsFrameWidget(mDestinationsTable, "Destinations", this);

        QList<DetailsFrameWidget*> framesSrcDstTab({mSourcesFrame, mDestinationsFrame});
        addTab("Src / Dst", framesSrcDstTab);

        //data tab
        mDataTable = new DataTableWidget(this);
        mDataFrame = new DetailsFrameWidget(mDataTable, "Data", this);

        addTab("Data", mDataFrame);

        //connect widgets with frames for refreshing the headers when necessary
        connect(mNetModuleTable, &NetModuleTable::updateText, mModulesFrame, &DetailsFrameWidget::setText);

        connect(mSourcesTable, &NetEndpointTable::updateText, mSourcesFrame, &DetailsFrameWidget::setText);
        connect(mDestinationsTable, &NetEndpointTable::updateText, mDestinationsFrame, &DetailsFrameWidget::setText);
    }
 
    void NetDetailsTabWidget::setNet(Net* net)
    {
        if (net) setIcon(SelectionDetailsIconProvider::NetIcon, net->get_id());
        //pass net or other stuff to widgets
        mNetInfoTable->setNet(net);
        mGroupingsOfItemTable->setNet(net);

        mNetModuleTable->setNet(net);
        mSourcesTable->setNet(net);
        mDestinationsTable->setNet(net);
        mDataTable->setNet(net);
    }
}
